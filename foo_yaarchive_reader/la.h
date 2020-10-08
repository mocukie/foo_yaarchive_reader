#pragma once

#include <SDK/foobar2000.h>
#include <helpers/readers.h>


#define LA_THROW_ERROR(a) throw exception_io_data(la::archive_error_string(a), la::archive_errno(a)); console::printf("[libarchive] error: %s", la::archive_error_string(a))
#define LA_MAKE_ARCHIVE_TYPE(T, ...) \
class T : public Format { \
public: \
	void extensions(pfc::array_t<const char*> & exts) override { \
		const char* arr[] = {__VA_ARGS__}; \
		exts.set_data_fromptr(arr,  PFC_TABSIZE(arr)); \
	}\
}


namespace larchive {
	namespace la {
#include <archive.h>
#include <archive_entry.h>
	}

	class Format {
	public:
		virtual void extensions(pfc::array_t<const char*> & exts) = 0;
	};

	LA_MAKE_ARCHIVE_TYPE(Zip, "zip");
	LA_MAKE_ARCHIVE_TYPE(Rar, "rar");
	LA_MAKE_ARCHIVE_TYPE(SevenZip, "7z");
	LA_MAKE_ARCHIVE_TYPE(Tar, "tar", "tar.gz", "tgz", "tar.xz", "txz");

	class Reader
	{
	private:
		file_ptr p_file;
		abort_callback* p_abort;

		la::archive* arch = nullptr;
		la::archive_entry* entry = nullptr;

		pfc::array_staticsize_t<uint8_t> block_buf;

		static la::la_ssize_t fb2k_read_cb(struct la::archive*, void* _client_data, const void** _buffer);
		static la::la_int64_t fb2k_skip_cb(struct la::archive*, void* _client_data, la::la_int64_t request);
		static int fb2k_close_cb(struct la::archive*, void* _client_data);
		static la::la_int64_t fb2k_seek_cb(struct la::archive*, void* _client_data, la::la_int64_t offset, int whence);

		inline la::archive* new_la_archive() {
			auto a = la::archive_read_new();

			la::archive_read_support_format_zip(a);
			la::archive_read_support_format_rar(a);
			la::archive_read_support_format_rar5(a);
			la::archive_read_support_format_7zip(a);
			la::archive_read_support_format_tar(a);
			la::archive_read_support_filter_xz(a);
			la::archive_read_support_filter_gzip(a);

			la::archive_read_set_callback_data(a, this);
			la::archive_read_set_read_callback(a, fb2k_read_cb);
			la::archive_read_set_skip_callback(a, fb2k_skip_cb);
			la::archive_read_set_close_callback(a, fb2k_close_cb);
			if (this->p_file->can_seek()) {
				la::archive_read_set_seek_callback(a, fb2k_seek_cb);
			}

			return a;
		};

		inline void release_la_archive() {
			if (arch) {
				la::archive_read_close(arch);
				la::archive_read_free(arch);
			}
		}

	public:

		Reader(file_ptr p_file) : p_file(p_file) {
			block_buf = pfc::array_staticsize_t<uint8_t>(65536);
			this->arch = new_la_archive();
			p_abort = &fb2k::noAbort;
		};

		~Reader() {
			release_la_archive();
		}

		void load(abort_callback& m_abort);

		bool next(abort_callback& m_abort);
		const char* pathname();
		t_filesize size();
		t_filetimestamp mTime();
		t_size data(void* buf, t_size, abort_callback& m_abort);
		int data_skip(abort_callback& m_abort);

		void reload(abort_callback& m_abort);
	};
}
