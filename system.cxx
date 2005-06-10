/*
 *  System specfic code for GeeXboX FLTK Generator
 *  Copyright (C) 2005  Amir Shalem
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "system.h"

#include <FL/Fl.H> /* Fl::check */

#ifdef __WIN32__
#include <windows.h> /* CreateProcess TerminateProcess */
#else
#include <sys/types.h>
#include <sys/wait.h> /* waitpid */
#include <signal.h> /* kill */
#include <paths.h> /* BSHELL path */
#endif

#ifdef __WIN32__
static PROCESS_INFORMATION pi;
static int bg_program = 0;
#else
static pid_t bg_pid = -1;
static int bg_status;
static void catch_bg_program(int sig)
{
    waitpid(bg_pid, &bg_status, 0);
    bg_pid = -1;
}
#endif

int execute_bg_program(char *string)
{
#ifdef __WIN32__
    STARTUPINFO si;
    DWORD exitcode;

    if (bg_program) /* someone is already running program in background */
	return -1;

    ZeroMemory (&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory (&pi, sizeof(pi));
    if (!CreateProcess(NULL, string, NULL, NULL, FALSE, 0 | CREATE_NO_WINDOW,
                       NULL, NULL, &si, &pi))
	return -1;

    bg_program++;
    while (WaitForSingleObject(pi.hProcess, 50) == WAIT_TIMEOUT)
    {
	Fl::check();
	if (!bg_program) /* destroy_bg_program was called while in Fl::check */
	    return -1;
    }
    bg_program--;

    GetExitCodeProcess(pi.hProcess, &exitcode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return (int)exitcode;
#else
    if (bg_pid != -1) /* someone is already running program in background */
	return -1;

    signal(SIGCHLD, catch_bg_program);

    if ((bg_pid = vfork()) < 0)
	return -1;

    if (!bg_pid)
    {
	execl(_PATH_BSHELL, "sh", "-c", string, (char*)NULL);
	_exit(127);
    }

    bg_status = 100;
    while (bg_pid != -1)
    {
	Fl::check();
	my_msleep(10);
    }

    return bg_status;
#endif
}

void destroy_bg_program(void)
{
#ifdef __WIN32__
    if (!bg_program)
	return;
    TerminateProcess(pi.hProcess, 100);
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    bg_program--;
#else
    if (bg_pid == -1)
	return;
    signal(SIGCHLD, SIG_DFL);
    kill(bg_pid, SIGKILL);
    catch_bg_program(SIGCHLD); /* wait & resets bg_pid */
#endif
}
