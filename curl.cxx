/*
 *  cURL wrapper code for GeeXboX FLTK Generator
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "curl.h"

#include <stdio.h>
#include <string.h> /* strcmp */
#include <unistd.h> /* unlink */
#include <stdlib.h> /* malloc, free */

#include <FL/Fl.H> 
#include <curl/curl.h>
#include <md5.h>
#include <bzlib.h>

static CURLM *multi_handle;

static void
stop_download(Fl_Widget *, void *data)
{
    *((int*)data) = 0;
}

static int
my_progress_func(void *data, double dltot, double dlnow, double ultotal, double ulnow)
{
    Fl_Progress *prog = (Fl_Progress*) data;

    if (dltot > 0)
    {
	prog->value(dlnow*100.0/dltot + (int)prog->user_data());
	Fl::check();
    }

    return 0;
}
										    
int download_progress(Fl_Button *b, Fl_Progress *prog, const char *url, curl_write_callback write_func, void *write_data)
{
    CURLMsg *msg; /* for picking up messages with the transfer status */
    int msgs_left; /* how many messages are left */
    int still_running = 1, ret;
    CURL *h;

    Fl_Callback* cb_func = NULL;
    void *cb_data = NULL;

    while ((msg = curl_multi_info_read(multi_handle, &msgs_left)))
	;

    if (b)
    {
	cb_func = b->callback();
	cb_data = b->user_data();

	b->callback(stop_download, &still_running);
    }

    h = curl_easy_init();

    curl_easy_setopt(h, CURLOPT_URL, url);
    curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(h, CURLOPT_WRITEDATA, write_data);

    if (prog)
    {
	curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, my_progress_func);
	curl_easy_setopt(h, CURLOPT_PROGRESSDATA, prog);
    }

    curl_multi_add_handle(multi_handle, h);

    while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running))
	;

    Fl::check();

    ret = 0;
    while (still_running)
    {
	struct timeval timeout;
	int rc;

	fd_set fdread;
	fd_set fdwrite;
	fd_set fdexcep;
	int maxfd;

	FD_ZERO(&fdread);
	FD_ZERO(&fdwrite);
	FD_ZERO(&fdexcep);

	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

	rc = ::select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
	if (rc > 0)
	    while(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running))
		;

	if (ret++ == 10) {
	    Fl::check();
	    ret = 0;
	}
    }

    ret = 1;
    while ((msg = curl_multi_info_read(multi_handle, &msgs_left)))
	if (msg->easy_handle == h && msg->msg == CURLMSG_DONE && msg->data.result == CURLE_OK)
	    ret = 0;

    curl_multi_remove_handle(multi_handle, h);
    curl_easy_cleanup(h);

    if (b)
	b->callback(cb_func, cb_data);

    return ret;
}

#define FILE_WRITE_MODE_MD5	0x001
#define FILE_WRITE_MODE_BZ2	0x002

#define BZ2_BUF_SIZE		0x10000

struct file_write_data {
    unsigned int mode;
    FILE *f;
    MD5_CTX md5_context;
    bz_stream bz2_strm;
    char *bz2_buf;
    int bz2_done;
};

static size_t
file_writefunction(char *buf, size_t size, size_t nmemb, void *stream)
{
    struct file_write_data *data = (struct file_write_data*) stream;
    size_t realsize = size * nmemb;

    if (data->mode & FILE_WRITE_MODE_MD5)
	MD5Update(&data->md5_context, (unsigned char*)buf, realsize);

    if (data->mode & FILE_WRITE_MODE_BZ2) {
	bz_stream *strm = &data->bz2_strm;
	size_t len;
	int ret;

	if (data->bz2_done)
	    return 0;

	strm->next_in = buf;
	strm->avail_in = realsize;

	for (;;)
	{
	    strm->next_out = data->bz2_buf;
	    strm->avail_out = BZ2_BUF_SIZE;

	    ret = BZ2_bzDecompress(strm);
	    if (ret == BZ_OK || ret == BZ_STREAM_END) {
		len = BZ2_BUF_SIZE - strm->avail_out;
		if (len)
		    if (fwrite(data->bz2_buf, sizeof(char), len, data->f) != len)
			return 0;
		if (ret == BZ_STREAM_END) {
		    data->bz2_done++;
		    if (strm->avail_in)
			return 0;
		}
		if (strm->avail_in == 0)
		    break;
	    } else {
		return 0;
	    }
	}
    } else {
	if (fwrite(buf, size, nmemb, data->f) != nmemb)
	    return 0;
    }

    return realsize;
}

static int match_end_clear(char *str, const char *end)
{
    size_t str_len, end_len;

    str_len = strlen(str);
    end_len = strlen(end);

    if (str_len >= end_len && !strcmp(&str[str_len - end_len], end)) {
	str[str_len - end_len] = '\0';
	return 1;
    } else {
	return 0;
    }
}

int download_file(Fl_Button *b, Fl_Progress *prog, const char *url, char *filename, const char *md5)
{
    struct file_write_data data;
    int ret = 2;

    data.mode = 0;

    if (md5) {
	data.mode |= FILE_WRITE_MODE_MD5;
	MD5Init(&data.md5_context);
    }

    if (match_end_clear(filename, ".bz2")) {
	data.mode |= FILE_WRITE_MODE_BZ2;
	data.bz2_done = 0;
	data.bz2_buf = (char*)malloc(BZ2_BUF_SIZE);
	data.bz2_strm.bzalloc = NULL;
	data.bz2_strm.bzfree = NULL;
	data.bz2_strm.opaque = NULL;
	BZ2_bzDecompressInit(&data.bz2_strm, 0, 0);
    }

    data.f = fopen(filename, "wb");
    if (data.f)
    {
	ret = download_progress(b, prog, url, file_writefunction, &data);
	fclose(data.f);

	if (!ret && data.mode & FILE_WRITE_MODE_MD5)
	{
	    char buf[33];
	    MD5End(&data.md5_context, buf);
	    if (md5 && strcmp(buf, md5))
		ret = 3;
	}

	if (!ret && data.mode & FILE_WRITE_MODE_BZ2 && !data.bz2_done)
	    ret = 4;

	if (ret)
	    unlink(filename);
    }

    if (data.mode & FILE_WRITE_MODE_BZ2) {
	free(data.bz2_buf);
	BZ2_bzDecompressEnd(&data.bz2_strm);
    }

    return ret;
}

int init_curl(void)
{
    multi_handle = curl_multi_init();
    if (!multi_handle)
	return 0;

    return 1;
}
