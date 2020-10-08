#include "pch.h"
#include "la.h"

namespace larchive {


	void Reader::load(abort_callback& m_abort)
	{
		this->p_abort = &m_abort;
		int code = la::archive_read_open1(this->arch);

		if (code != ARCHIVE_OK) {
			LA_THROW_ERROR(arch);
		}
	}

	bool larchive::Reader::next(abort_callback& m_abort)
	{
		this->p_abort = &m_abort;
		int ret = la::archive_read_next_header(arch, &entry);
		if (ret == ARCHIVE_OK) {
			return true;
		}
		else if (ret == ARCHIVE_EOF) {
			return false;
		}

		LA_THROW_ERROR(arch);
	}

	const char* larchive::Reader::pathname()
	{
		const char* utf8cs = la::archive_entry_pathname_utf8(entry);
		if (utf8cs)
			return utf8cs;

		const wchar_t* wcs = la::archive_entry_pathname_w(entry);
		if (wcs) {
			pfc::array_staticsize_t<char> utf8;
			auto len = wcslen(wcs);
			utf8.set_size_discard(pfc::stringcvt::estimate_wide_to_utf8(wcs, len));
			pfc::stringcvt::convert_wide_to_utf8(utf8.get_ptr(), utf8.size(), wcs, len);
			archive_entry_set_pathname_utf8(entry, utf8.get_ptr());
			return  la::archive_entry_pathname_utf8(entry);
		}

		return la::archive_entry_pathname(entry);
	}


	t_filesize larchive::Reader::size()
	{
		if (la::archive_entry_size_is_set(entry)) {
			return (t_filesize)la::archive_entry_size(entry);
		}
		else {
			return foobar2000_io::filesize_invalid;
		}
	}

	t_filetimestamp larchive::Reader::mTime()
	{
		if (la::archive_entry_mtime_is_set(entry)) {
			//The time_t on mingw is int32
			t_filetimestamp mtime = (int32_t)la::archive_entry_mtime(entry);
			//number of seconds from 1601/1/1 00:00 to 1970/1/1 00:00 UTC
			t_filetimestamp stamp = 11644473600ull;
			stamp += mtime;
			stamp *= foobar2000_io::filetimestamp_1second_increment;
			stamp += la::archive_entry_mtime_nsec(entry) / 100ull;
			return stamp;
		}
		else {
			return foobar2000_io::filetimestamp_invalid;
		}

	}

	t_size larchive::Reader::data(void* buf, t_size size, abort_callback& m_abort)
	{
		this->p_abort = &m_abort;
		auto r = la::archive_read_data(arch, buf, size);
		if (r < 0) {
			LA_THROW_ERROR(arch);
		}
		return (t_size)r;
	}

	int larchive::Reader::data_skip(abort_callback& m_abort)
	{
		this->p_abort = &m_abort;
		int r = la::archive_read_data_skip(arch);
		return r;
	}

	void Reader::reload(abort_callback& m_abort)
	{
		this->p_file->reopen(m_abort);
		release_la_archive();
		this->arch = new_la_archive();
		load(m_abort);
	}

	//callback
	la::la_ssize_t larchive::Reader::fb2k_read_cb(la::archive*, void* _client_data, const void** _buffer)
	{
		auto data = reinterpret_cast<larchive::Reader*>(_client_data);
		*_buffer = data->block_buf.get_ptr();
		return data->p_file->read(data->block_buf.get_ptr(), data->block_buf.get_size(), *data->p_abort);
	}

	la::la_int64_t larchive::Reader::fb2k_skip_cb(la::archive*, void* _client_data, la::la_int64_t request)
	{
		auto data = reinterpret_cast<larchive::Reader*>(_client_data);
		return data->p_file->skip(request, *data->p_abort);
	}

	int larchive::Reader::fb2k_close_cb(la::archive*, void* _client_data)
	{
		return ARCHIVE_OK;
	}

	la::la_int64_t larchive::Reader::fb2k_seek_cb(la::archive*, void* _client_data, la::la_int64_t offset, int whence)
	{
		auto data = reinterpret_cast<larchive::Reader*>(_client_data);
		file::t_seek_mode mode;
		switch (whence) {
		case SEEK_SET:
			mode = file::t_seek_mode::seek_from_beginning;
			break;
		case SEEK_CUR:
			mode = file::t_seek_mode::seek_from_current;
			break;
		case SEEK_END:
			mode = file::t_seek_mode::seek_from_eof;
			break;
		default:
			return ARCHIVE_FATAL;
		}

		try {
			data->p_file->seek_ex(offset, mode, *data->p_abort);
			return data->p_file->get_position(*data->p_abort);
		}
		catch (...) {
			return ARCHIVE_FATAL;
		}
	}

}