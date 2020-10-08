#pragma once

#include <memory>
#include <SDK/foobar2000.h>
#include <helpers/readers.h>
#include <helpers\seekabilizer.h>

#include "la.h"

class yala_file_impl_t;

class NOVTABLE yala_file : public file_readonly
{
public:

	virtual void load(const char* _archive, const char* _entry, abort_callback& p_abort) {
		file_ptr f;
		filesystem::g_open(f, _archive, filesystem::open_mode_read, p_abort);
		load(f, _entry, p_abort);
	};

	virtual void load(service_ptr_t<file>_archive, const char* _entry, abort_callback& p_abort) = 0;

	static file_ptr new_yala_file(
		const char* _archive, const char* _entry,
		abort_callback& p_abort, t_size p_buffer_size = 1024 * 1024 * 16);

public:
	FB2K_MAKE_SERVICE_INTERFACE(yala_file, file);
};


class yala_file_impl_t : public yala_file
{

private:
	pfc::array_staticsize_t<char> entry;
	t_filesize position = 0;
	bool _remote = false;
	std::unique_ptr<larchive::Reader> reader;
	void seek_to_entry(abort_callback& p_abor);

	//yala_file
public:
	void load(service_ptr_t<file>_archive, const char* _entry, abort_callback& p_abort) override;

	//file
public:
	t_filesize get_size(abort_callback& p_abort) override;
	t_filesize get_position(abort_callback& p_abort) override;
	void seek(t_filesize p_position, abort_callback& p_abort) override;
	void seek_ex(t_sfilesize p_position, file::t_seek_mode p_mode, abort_callback& p_abort) override;
	bool can_seek() override { return false; };
	bool get_content_type(pfc::string_base&) override { return false; }
	bool is_in_memory() override { return false; };
	t_filetimestamp get_timestamp(abort_callback& p_abort) override;
	void reopen(abort_callback& p_abort) override;
	bool is_remote() override { return _remote; };

	//stream_reader
public:
	t_size read(void* p_buffer, t_size p_bytes, abort_callback& p_abort) override;
	void read_object(void* p_buffer, t_size p_bytes, abort_callback& p_abort) override;
	t_filesize skip(t_filesize p_bytes, abort_callback& p_abort) override;
	void skip_object(t_filesize p_bytes, abort_callback& p_abort) override;

};
