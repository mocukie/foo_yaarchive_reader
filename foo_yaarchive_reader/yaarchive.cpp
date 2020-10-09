#include "pch.h"

#include "la.h"
#include "yala_file.h"


template<typename F>
class yaarchive : public foobar2000_io::archive_impl {

private:
	pfc::array_t<const char*> extensions;
public:
	yaarchive() {
		F format;
		format.extensions(extensions);
	}

	bool supports_content_types() override
	{
		return false;
	}

	const char* get_archive_type() override
	{
		return extensions[0];
	}

	t_filestats get_stats_in_archive(const char* p_archive, const char* p_file, abort_callback& p_abort)
	{
		service_ptr_t<file> m_file;
		filesystem::g_open(m_file, p_archive, filesystem::open_mode_read, p_abort);

		larchive::Reader rd(m_file);
		rd.load(p_abort);

		while (rd.next(p_abort)) {
			const char* entry_path = rd.pathname();
			if (!strcmp(p_file, entry_path)) {
				t_filestats ret;
				ret.m_size = rd.size();
				ret.m_timestamp = rd.mTime();
				return ret;
			}

			rd.data_skip(p_abort);
		}

		throw exception_io_not_found();
	}

	void open_archive(service_ptr_t< file >& p_out, const char* p_archive, const char* p_file, abort_callback& p_abort)
	{
		service_ptr_t<file> m_file;
		filesystem::g_open(m_file, p_archive, filesystem::open_mode_read, p_abort);

		larchive::Reader rd(m_file);
		rd.load(p_abort);

		while (rd.next(p_abort)) {
			const char* entry_path = rd.pathname();
			if (!strcmp(p_file, entry_path)) {
				p_out = yala_file::new_yala_file(p_archive, entry_path, p_abort);
				return;
			}

			rd.data_skip(p_abort);
		}
		throw exception_io_not_found();

	}

	void archive_list(const char* path, const service_ptr_t<file>& p_reader, archive_callback& p_out, bool p_want_readers) {

		service_ptr_t<file> m_file = p_reader;
		if (m_file.is_empty())
			filesystem::g_open(m_file, path, filesystem::open_mode_read, p_out);
		else
			m_file->reopen(p_out);

		larchive::Reader rd(m_file);
		rd.load(p_out);

		pfc::string8_fastalloc upack_path;
		while (rd.next(p_out)) {
			const char* entry_path = rd.pathname();
			make_unpack_path(upack_path, path, entry_path);

			file_ptr out_file;
			t_filestats stats;
			stats.m_size = rd.size();
			stats.m_timestamp = rd.mTime();
			if (p_want_readers) {
				out_file = yala_file::new_yala_file(path, entry_path, p_out, 0);
			}
			rd.data_skip(p_out);
			if (!p_out.on_entry(this, upack_path, stats, out_file))
				break;
		}
	}

	bool is_our_archive(const char* path)
	{
		if (g_is_unpack_path(path)) {
			return false;
		}

		pfc::string_extension ext(path);
		for (size_t i = 0; i < extensions.size(); ++i) {
			if (!_stricmp(extensions[i], ext))
				return true;
			;
		}
		return false;
	}

};



static archive_factory_t <yaarchive<larchive::Zip>> g_yaar_zip_factory;
static archive_factory_t <yaarchive<larchive::Rar>> g_yaar_rar_factory;
static archive_factory_t <yaarchive<larchive::SevenZip>> g_yaar_7z_factory;
static archive_factory_t <yaarchive<larchive::Tar>> g_yaar_tar_factory;

namespace {
	static service_factory_single_t<input_file_type_v2_impl> g_zip("zip", "ZIP Archive", "ZIP Archives");
	static service_factory_single_t<input_file_type_v2_impl> g_rar("rar", "RAR Archive", "RAR Archives");
	static service_factory_single_t<input_file_type_v2_impl> g_7z("7z", "7-Zip Archive", "7-Zip Archives");
	static service_factory_single_t<input_file_type_v2_impl> g_tar("tar;tar.gz;tgz;tar.xz;txz", "Tape Archive", "Tape Archive");
}

DECLARE_COMPONENT_VERSION("Yet Another Archive Reader", "0.0.1", 
R"(Read audio in zip, rar, 7z, tar directly without upack to memory or temp folder.

Copyright (C) 2020 Marklin Lin <mocukie0@gmail.com>
)"
);
VALIDATE_COMPONENT_FILENAME("foo_yaarchive_reader.dll");
