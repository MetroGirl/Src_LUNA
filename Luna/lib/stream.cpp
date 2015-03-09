#include "stdafx.h"
#include "stream.h"

namespace luna{
	LUNA_IMPLEMENT_ABSTRACT(luna::Stream);
	LUNA_IMPLEMENT_CONCRETE(luna::FileStream);
	LUNA_IMPLEMENT_CONCRETE(luna::PackStream);

	Stream::~Stream()
	{
	}

	FileStream::FileStream()
	{
		LUNA_ASSERT(0, "do not create instance via new. use File::createFileStream() instead.");
	}

	FileStream::FileStream(File* filePtr)
		: mFilePtr(filePtr)
	{
	}

	FileStream::~FileStream()
	{
		if (mFilePtr){
			mFilePtr->close();
		}
	}

	size_t FileStream::read(void* bufferPtr, size_t byte)
	{
		return mFilePtr->read(bufferPtr, byte);
	}

	size_t FileStream::write(void* bufferPtr, size_t byte)
	{
		return mFilePtr->write(bufferPtr, byte);
	}

	size_t FileStream::seek(s64 offset, File::SeekMode mode)
	{
		return mFilePtr->seek(offset, mode);
	}

	bool FileStream::isReadable()
	{
		return mFilePtr->isOpened();
	}

	bool FileStream::isWritable()
	{
		return mFilePtr->isOpened();
	}

	size_t FileStream::getSize()
	{
		return mFilePtr->getSize();
	}

	const wstring& FileStream::getDir()
	{
		return mFilePtr->getDir();
	}

	const wstring& FileStream::getPath()
	{
		return mFilePtr->getPath();
	}

	const wstring& FileStream::getExtension()
	{
		return mFilePtr->getExtension();
	}


	PackStream::PackStream()
	{
		LUNA_ASSERT(0, "do not create instance via new. use File::createPackStream() instead.");
	}

	PackStream::PackStream(const wstring& dir, const wstring& path, const wstring& extension, File* filePtr, size_t startOffset, size_t sizeStream)
		: mDir(dir)
		, mPath(path)
		, mExtension(extension)
		, mFilePtr(filePtr)
		, mStartOffset(startOffset)
		, mSize(sizeStream)
	{
		mFilePtr->seek(mStartOffset, File::SeekMode_Set);
	}

	PackStream::~PackStream()
	{
	}

	size_t PackStream::read(void* bufferPtr, size_t byte)
	{
		return mFilePtr->read(bufferPtr, byte);
	}

	size_t PackStream::write(void* bufferPtr, size_t byte)
	{
		return 0;
	}

	size_t PackStream::seek(s64 offset, File::SeekMode mode)
	{
		s64 targetOffset = 0;
		switch (mode)
		{
		case luna::File::SeekMode_Set:
			targetOffset = ((s64)mStartOffset) + offset;
			mode = File::SeekMode_Set;
			break;
		case luna::File::SeekMode_Cur:
			targetOffset = offset;
			mode = File::SeekMode_Cur;
			break;
		case luna::File::SeekMode_End:
			targetOffset = ((s64)(mStartOffset + mSize)) + offset;
			mode = File::SeekMode_Set;
			break;
		default:
			break;
		}
		return mFilePtr->seek(targetOffset, mode);
	}

	bool PackStream::isReadable()
	{
		return mFilePtr->isOpened();
	}

	bool PackStream::isWritable()
	{
		return false;
	}

	size_t PackStream::getSize()
	{
		return mSize;
	}

	const wstring& PackStream::getDir()
	{
		return mDir;
	}

	const wstring& PackStream::getPath()
	{
		return mPath;
	}

	const wstring& PackStream::getExtension()
	{
		return mExtension;
	}


}