#include <unistd.h>
#include "audio_capture_manager.h"
#include "music_id.h"
#include "acm_iarm_interface.h"
void launcher()
{
	audio_properties_t settings = {16, 48000, 2, 0};
	q_mgr manager(settings);
	music_id_client client(&manager);
	acm_iarm_interface * iface = acm_iarm_interface::get_instance();
	iface->activate(&client);

	/* Hold here until application is terminated.*/
	pause();
	iface->deactivate();
}


int main(int argc, char *argv[])
{
	launcher();
	return 0;
}
