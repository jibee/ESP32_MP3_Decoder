/* ****************************************************************************
 *                                                                            *
 * HttpClient.hpp                                                             *
 * Description                                                                *
 *                                                                            *
 * Function                                                                   *
 *                                                                            *
 *****************************************************************************/
/* ****************************************************************************
 *                                                                            *
 * Created on: 2017-07-10T20:37:26                                            *
 * 4.4.0-83-generic #106-Ubuntu SMP Mon Jun 26 17:54:43 UTC 2017              *
 * Revisions:                                                                 *
 *                                                                            *
 *****************************************************************************/
/******************************************************************************
 *   This program is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by     *
 *   the Free Software Foundation; either version 2 of the License, or        *
 *   (at your option) any later version.                                      *
 ******************************************************************************

 ******************************************************************************
 *  Contributors:                                                             *
 * (c) 2007 Jean-Baptiste Mayer (jibee@jibee.com) (initial work)              *
 *  Name/Pseudo <email>                                                       *
 ******************************************************************************/

#ifndef HTTPCLIENT_HPP
#define HTTPCLIENT_HPP

#include <string>
struct http_parser;

class HttpClient
{
    public:
	void start();
	virtual int on_status(const char *at, size_t length);
	virtual int on_header_field(const char *at, size_t length);
	virtual int on_header_value(const char *at, size_t length);
	virtual int on_message_complete();
	virtual int on_body(const char *at, size_t length);
	virtual int on_headers_complete();
	virtual void http_get_task();

	HttpClient(const std::string& url);
	virtual ~HttpClient();
	std::string getUrl() const { return url; };
    private:
	std::string url;
	static int on_status_cb(http_parser* parser, const char *at, size_t length);
	static int on_header_field_cb(http_parser *parser, const char *at, size_t length);
	static int on_header_value_cb(http_parser *parser, const char *at, size_t length);
	static int on_message_complete_cb(http_parser *parser);
	static int on_body_cb(http_parser* parser, const char *at, size_t length);
	static int on_headers_complete_cb(http_parser *parser);
	static void http_get_task(void *pvParameters);
};

#endif /* HTTPCLIENT_HPP */

