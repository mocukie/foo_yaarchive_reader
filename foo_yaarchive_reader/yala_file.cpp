#include "pch.h"
#include "yala_file.h"

const GUID yala_file::class_guid = { 0x6c47b80e, 0x473d, 0x4107, { 0x87, 0x04 , 0x16,0x02,0x63,0xdb,0xb3,0xb6 } };


file_ptr yala_file::new_yala_file(const char* _archive, const char* _entry, abort_callback& p_abort, t_size p_buffer_size)
{
	service_ptr_t<yala_file> yala = fb2k::service_new<yala_file_impl_t>();
	yala->load(_archive, _entry, p_abort);
	if (p_buffer_size > 0) {
		auto seekable = fb2k::service_new<seekabilizer>();
		seekable->initialize(yala, p_buffer_size, p_abort);
		return seekable;
	}
	return yala;
}


void yala_file_impl_t::seek_to_entry(abort_callback& p_abort) {

	while (reader->next(p_abort)) {
		const char* pathname = reader->pathname();
		if (!strcmp(entry.get_ptr(), pathname)) {
			return;
		}
		reader->data_skip(p_abort);
	}
	console::printf("archive entry '%s' not found\n", entry.get_ptr());
	throw exception_io_not_found();
}


void yala_file_impl_t::load(service_ptr_t<file> _archive, const char* _entry, abort_callback& p_abort)
{
	_remote = _archive->is_remote();
	reader = std::unique_ptr<larchive::Reader>(new larchive::Reader(_archive));
	entry = pfc::array_staticsize_t<char>(strlen(_entry) + 1);
	memcpy(entry.get_ptr(), _entry, entry.get_size());

	reader->load(p_abort);
	seek_to_entry(p_abort);
}

t_filesize yala_file_impl_t::get_size(abort_callback& p_abort)
{
	return reader->size();
}

t_filesize yala_file_impl_t::get_position(abort_callback& p_abort)
{
	return position;
}

void yala_file_impl_t::seek(t_filesize p_position, abort_callback& p_abort)
{
	throw exception_io_object_not_seekable();
}

void yala_file_impl_t::seek_ex(t_sfilesize p_position, file::t_seek_mode p_mode, abort_callback& p_abort)
{
	throw exception_io_object_not_seekable();
}

t_filetimestamp yala_file_impl_t::get_timestamp(abort_callback& p_abort)
{
	return reader->mTime();
}

void yala_file_impl_t::reopen(abort_callback& p_abort)
{
	position = 0;
	reader->reload(p_abort);
	seek_to_entry(p_abort);
}

t_size yala_file_impl_t::read(void* p_buffer, t_size p_bytes, abort_callback& p_abort)
{
	t_size r = reader->data(p_buffer, p_bytes, p_abort);
	position += r;
	return r;
}

void yala_file_impl_t::read_object(void* p_buffer, t_size p_bytes, abort_callback& p_abort)
{
	if (p_bytes > get_remaining(p_abort))
		throw exception_io_data_truncation();

	t_size r = reader->data(p_buffer, p_bytes, p_abort);
	position += r;
}

t_filesize yala_file_impl_t::skip(t_filesize p_bytes, abort_callback& p_abort)
{
	t_filesize remaining = get_remaining(p_abort);
	t_filesize r = p_bytes >= remaining ? remaining : p_bytes;

	pfc::array_staticsize_t<uint8_t> buf(r);
	r = reader->data(buf.get_ptr(), r, p_abort);
	position += r;
	return r;
}

void yala_file_impl_t::skip_object(t_filesize p_bytes, abort_callback& p_abort)
{
	if (p_bytes > get_remaining(p_abort))
		throw exception_io_data_truncation();

	pfc::array_staticsize_t<uint8_t> buf(p_bytes);
	auto r = reader->data(buf.get_ptr(), p_bytes, p_abort);
	position += r;
}

