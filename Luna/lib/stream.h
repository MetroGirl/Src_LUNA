#ifndef LUNA_STREAM_H_INCLUDED
#define LUNA_STREAM_H_INCLUDED

#include "lib/type.h"
#include "lib/file.h"

namespace luna{
	class Stream : public Object{
		LUNA_DECLARE_ABSTRACT(Stream, Object);

	public:
		Stream(){}
		virtual ~Stream();

		virtual size_t read(void* bufferPtr, size_t byte) = 0;
		virtual size_t write(void* bufferPtr, size_t byte) = 0;

		virtual bool isReadable() = 0;
		virtual bool isWritable() = 0;

		virtual size_t getSize() = 0;

		virtual const wstring& getDir() = 0;
		virtual const wstring& getPath() = 0;
		virtual const wstring& getExtension() = 0;

		virtual size_t seek(s64 offset, File::SeekMode mode) = 0;

	public:
		vector<c8> readByteString()
		{
			vector<c8> buffer(getSize());
			read(buffer.data(), buffer.size());
			return buffer;
		}

	private:
	};

	class FileStream : public Stream{
		LUNA_DECLARE_CONCRETE(FileStream, Stream);

	public:
		FileStream(File* filePtr);
		virtual ~FileStream();

		size_t read(void* bufferPtr, size_t byte);
		size_t write(void* bufferPtr, size_t byte);

		bool isReadable();
		bool isWritable();

		size_t getSize();

		const wstring& getDir();
		const wstring& getPath();
		const wstring& getExtension();

		size_t seek(s64 offset, File::SeekMode mode);

	private:
		FileStream();

	protected:
		File* mFilePtr;
	};

	class PackStream : public Stream{
		LUNA_DECLARE_CONCRETE(PackStream, Stream);

	public:
		PackStream(const wstring& dir, const wstring& path, const wstring& extension, File* filePtr, size_t startOffset, size_t sizeStream);
		virtual ~PackStream();

		size_t read(void* bufferPtr, size_t byte);
		size_t write(void* bufferPtr, size_t byte);

		bool isReadable();
		bool isWritable();

		size_t getSize();

		const wstring& getDir();
		const wstring& getPath();
		const wstring& getExtension();

		size_t seek(s64 offset, File::SeekMode mode);

	private:
		PackStream();

	protected:
		wstring mDir;
		wstring mPath;
		wstring mExtension;
		size_t mStartOffset;
		size_t mSize;
		File* mFilePtr;
	};

}

#endif // LUNA_STREAM_H_INCLUDED
