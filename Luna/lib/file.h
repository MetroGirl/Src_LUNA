#ifndef LUNA_FILE_H_INCLUDED
#define LUNA_FILE_H_INCLUDED

namespace luna{
	class File : public Object{
		LUNA_DECLARE_CONCRETE(File, Object);

	public:
		enum OpenMode
		{
			OpenMode_Read,
			OpenMode_ReadWrite,
		};

		enum OptionFlag
		{
			OptionFlag_None = 0,
			OptionFlag_Truncate = 1,
			OptionFlag_Async = 2,
			OptionFlag_NoRecording = 4,
			OptionFlag_ByPassPack = 8,
		};

		enum SeekMode
		{
			SeekMode_Set,
			SeekMode_Cur,
			SeekMode_End,
		};

	public:
		File();
		virtual ~File();

		bool open(const c8* fileName, OpenMode mode, OptionFlag flags = OptionFlag_None);
		bool open(const c16* fileName, OpenMode mode, OptionFlag flags = OptionFlag_None);
		void close();

		size_t read(void* bufferPtr, size_t sizeToRead);
		size_t write(const void* bufferPtr, size_t sizeToWrite);
		size_t seek(s64 offset, SeekMode mode);

		bool isOpened() const;
		size_t getSize() const
		{ 
			return mSize;
		}

		const wstring& getDir() const
		{
			return mDir;
		}

		const wstring& getPath() const
		{
			return mPath;
		}

		const wstring& getExtension() const
		{
			return mExtension;
		}

		FILETIME getCurrentTime() const;

		static bool isNewer(const FILETIME& yours, const FILETIME& theirs);

		static bool exist(const c16* fileName);
		static bool copy(const c16* srcFileName, const c16* dstFileName, bool isOverWrite);
		static bool move(const c16* srcFileName, const c16* dstFileName);
		static bool remove(const c16* fileName);

		static void beginRecording();
		static void endRecording(vector<pair<FILETIME,wstring>>& tbl);

	public:
		struct PackHeader
		{
			u32 magic;
			u32 version;
			u32 tocCount;
			u32 reserved;
		};
		struct PackToc
		{
			size_t offset;
			size_t size;
			c16 pathStr[64];
			c16 dirStr[64];
			c16 extStr[16];
			u32 timeLow;
			u32 timeHigh;
		};
		static void writePackFile(const c16* path);
		static bool readPackFile(const c16* path);

		static const PackHeader& getPackHeader()
		{
			return mPackHeader;
		}

		static const vector<PackToc>& getPackToc()
		{
			return mPackToc;
		}

		static File& getPackFile()
		{
			return mPackFile;
		}

	private:
		size_t mSize;
#if LUNA_WINDOWS
		HANDLE mFile;
#endif
		PackToc* mPackHandle;
		size_t mPackPt;
		wstring mDir;
		wstring mPath;
		wstring mExtension;

		static s32 mRecordingPt;
		static array<vector<pair<FILETIME, wstring>>, 64> mRecordingTblTbl;
		static vector<wstring> mPackRecordingTbl;
		static PackHeader mPackHeader;
		static vector<PackToc> mPackToc;
		static File mPackFile;
	};
}

#endif // LUNA_FILE_H_INCLUDED
