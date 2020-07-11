#include "YoloAlgVx.h"

int yolo_test(int camera_index) 
{
	int cpus = sysconf(_SC_NPROCESSORS_CONF);

	array<thread, 4> threads;
	threads = {thread(cameraRead, camera_index),thread(displayImage),thread(run_process, 0),thread(run_process, 1)};

	for (int i = 0; i < 4; i++)
	{
		threads[i].join();
	}
		
	return 0;

}
