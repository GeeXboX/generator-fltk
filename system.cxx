/*
 *  System specfic code for GeeXboX FLTK Generator
 *  Copyright (C) 2005-2006  Amir Shalem
 *  Copyright (C) 2008  Mathieu Schroeter
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "system.h"

#include <FL/Fl.H> /* Fl::check */

#ifdef _WIN32
#include <windows.h> /* CreateProcess TerminateProcess */
#include <stdio.h> /* snprintf */
#else
#include <sys/types.h>
#include <sys/wait.h> /* waitpid */
#include <signal.h> /* kill */
#include <unistd.h> /* vfork execl _exit */
#include <paths.h> /* BSHELL path */
#endif

#ifdef _WIN32
static PROCESS_INFORMATION pi;
static int bg_program = 0;
#else
static pid_t bg_pid = -1;
static int bg_status;
#endif

int execute_bg_program(char *string)
{
#ifdef _WIN32
    STARTUPINFO si;
    DWORD exitcode;
    char cmd[2048];

    if (bg_program) /* someone is already running program in background */
        return -1;

    ZeroMemory (&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory (&pi, sizeof(pi));
    snprintf(cmd, sizeof(cmd), "cmd.exe /c %s", string);
    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0 | CREATE_NO_WINDOW,
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

    if ((bg_pid = vfork()) < 0)
        return -1;

    if (!bg_pid)
    {
        execl(_PATH_BSHELL, "sh", "-c", string, (char*)NULL);
        _exit(127);
    }

    bg_status = 100;
    while (!waitpid(bg_pid, &bg_status, WNOHANG))
    {
        Fl::check();
        my_msleep(10);
    }
    bg_pid = -1;

    return bg_status;
#endif
}

void destroy_bg_program(void)
{
#ifdef _WIN32
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
    kill(bg_pid, SIGKILL);
    waitpid(bg_pid, &bg_status, 0);
    bg_pid = -1;
#endif
}
