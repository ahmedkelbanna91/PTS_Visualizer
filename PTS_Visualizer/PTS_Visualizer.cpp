#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <windows.h>


// Define a custom point type for XYZUVW
struct PointXYZUVW {
	float x, y, z, u, v, w;
};

POINT_CLOUD_REGISTER_POINT_STRUCT(PointXYZUVW, (float, x, x) (float, y, y) (float, z, z) (float, u, u) (float, v, v) (float, w, w))

// Function to read .pts file
bool readPTSFile(const std::string& filename, pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_xyz,
	pcl::PointCloud<PointXYZUVW>::Ptr cloud_xyzuvw) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open file " << filename << std::endl;
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::vector<float> values;
		float value;
		while (iss >> value) {
			values.push_back(value);
		}

		if (values.size() == 3) {
			cloud_xyz->push_back(pcl::PointXYZ(values[0], values[1], values[2]));
		}
		else if (values.size() == 6) {
			PointXYZUVW point;
			point.x = values[0];
			point.y = values[1];
			point.z = values[2];
			point.u = values[3];
			point.v = values[4];
			point.w = values[5];
			cloud_xyzuvw->push_back(point);
		}
		else {
			std::cerr << "Invalid line in PTS file: " << line << std::endl;
		}
	}

	file.close();
	return true;
}

int main(int argc, char** argv) {
	SetDllDirectory(L"libs");
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <pts_file>" << std::endl;
		return -1;
	}
	
	std::string filename = argv[1];  // Get the filename from the command line
	//std::string filename = "C:/Users/kline/Desktop/DEV/PTS_Visualizer/x64/Release/Haze_Model_fixture-2.pts";

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_xyz(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<PointXYZUVW>::Ptr cloud_xyzuvw(new pcl::PointCloud<PointXYZUVW>);

	if (!readPTSFile(filename, cloud_xyz, cloud_xyzuvw)) {
		return -1;
	}

	// Visualize the point cloud
	pcl::visualization::PCLVisualizer viewer("3D Viewer");
	// Set the background to grey
	viewer.setBackgroundColor(0.2, 0.2, 0.2);
	viewer.addCoordinateSystem(5.0);  // Scale of the coordinate system

	// Parameters: camera position (x, y, z), focal point (x, y, z), view up (x, y, z)
	//viewer.setCameraPosition(0, 0, 0, 0, -1, -1, 0, 1, 0);
	double distance = 2.0; // Distance from the point cloud, adjust as needed
	double cameraYOffset = distance * std::sqrt(2) / 2; // Y offset for 45 degrees
	double cameraZOffset = -distance * std::sqrt(2) / 2; // Negative Z offset for front view

	viewer.setCameraPosition(
		0, cameraYOffset, cameraZOffset,  // Camera position (X=0, Y and Z calculated for 45 degrees up and front view)
		0, 0, 1,  // Focal point at the center top of the cloud (assuming cloud centered around origin)
		0, 0, 1  // Up vector (negative Y to correct flip)
	);


	double xyz_sphere_radius = 0.3, UVW_sphere_radius = 0.3;  // Adjusted sphere radius for visibility

	// Adding XYZ points as spheres with unique IDs
	for (size_t i = 0; i < cloud_xyz->points.size(); ++i) {
		const auto& point = cloud_xyz->points[i];
		std::string sphere_id = "XYZ_sphere_" + std::to_string(i);
		viewer.addSphere(point, xyz_sphere_radius, 255, 0, 0, sphere_id);
	}

	// Adding XYZUVW points and normals with unique IDs
	for (size_t i = 0; i < cloud_xyzuvw->points.size(); ++i) {
		const auto& point = cloud_xyzuvw->points[i];
		pcl::PointXYZ pcl_point(point.x, point.y, point.z);
		pcl::PointXYZ pcl_normal(point.x + (point.u * 2.0),  // Adjust normal scale
			point.y + (point.v * 2.0),
			point.z + (point.w * 2.0));

		std::string sphere_id = "UVW_sphere_" + std::to_string(i);
		viewer.addSphere(pcl_point, UVW_sphere_radius, 0, 255, 0, sphere_id);

		std::string line_id = "normal_" + std::to_string(i);
		viewer.addLine(pcl_point, pcl_normal, 255, 0, 0, line_id);
		viewer.setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_LINE_WIDTH, 3, line_id);
	}



	viewer.resetCamera();

	// Main visualization loop
	while (!viewer.wasStopped()) {
		viewer.spinOnce(100);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}





//#include <vtkSTLReader.h>
//#include <vtkPolyDataMapper.h>
//#include <vtkActor.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkSmartPointer.h>
//#include <iostream>
//
//int main(int argc, char** argv) {
//    if (argc < 2) {
//        std::cerr << "Usage: " << argv[0] << " <stl_file>" << std::endl;
//        return -1;
//    }
//
//    std::string stl_filename = argv[1];
//
//    // Read STL file using VTK
//    vtkSmartPointer<vtkSTLReader> stlReader = vtkSmartPointer<vtkSTLReader>::New();
//    stlReader->SetFileName(stl_filename.c_str());
//    stlReader->Update();
//
//    // Check if STL file is loaded
//    if (stlReader->GetOutput()->GetNumberOfPoints() == 0) {
//        std::cerr << "Error: No points read from STL file" << std::endl;
//        return -1;
//    }
//    else {
//        std::cout << "STL file loaded successfully. Number of points: " << stlReader->GetOutput()->GetNumberOfPoints() << std::endl;
//    }
//
//    // Create a mapper
//    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
//    mapper->SetInputConnection(stlReader->GetOutputPort());
//
//    // Create an actor
//    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
//    actor->SetMapper(mapper);
//
//    // Create a renderer, render window, and interactor
//    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
//    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
//    renderWindow->AddRenderer(renderer);
//    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//    renderWindowInteractor->SetRenderWindow(renderWindow);
//
//    // Add the actor to the scene
//    renderer->AddActor(actor);
//    renderer->SetBackground(0.3, 0.2, 0.1); // Background color dark grey
//
//    // Render and interact
//    std::cout << "Rendering STL file..." << std::endl;
//    renderWindow->Render();
//    renderWindowInteractor->Start();
//
//    return 0;
//}





