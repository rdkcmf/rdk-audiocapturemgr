/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
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
