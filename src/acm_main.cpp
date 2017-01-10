#include <unistd.h>
#include "audio_capture_manager.h"
#include "music_id.h"
#include "acm_session_mgr.h"
void launcher()
{
	acm_session_mgr *mgr = acm_session_mgr::get_instance();
	mgr->activate();
	/* Hold here until application is terminated.*/
	pause();
	mgr->deactivate();
}


int main(int argc, char *argv[])
{
	launcher();
	return 0;
}
