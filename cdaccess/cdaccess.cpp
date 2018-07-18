// cdaccess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cdaccess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LOG_ERROR       1
#define LOG_WARNING     2
#define LOG_INFO        3
#define LOG_VERBOSE     4

#define Log(level, format, ...) _Log(level, format ## "\r\n", __VA_ARGS__)
void _Log(LONG level, const char *format, ...)
{
    va_list list;
    char buffer[256];
    va_start(list, format);
    vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, list);
    printf(buffer);
    va_end(list);
}

// The one and only application object

CWinApp theApp;

using namespace std;

class tConfig
{
public:
    tConfig(int argc, TCHAR **argv);
    CString device;
    CString file;
    ULONG parameter;
    union {
        ULONG val;
        struct {
            ULONG fList   : 1;
            ULONG fRead   : 1;
            ULONG fCheck  : 1;
            ULONG fEject  : 1;
            ULONG fInsert : 1;
            ULONG fToc    : 1;
            ULONG fGeometry : 1;
            ULONG fSeek : 1;
            ULONG fParam : 1;
            ULONG fOutFile : 1;
            ULONG fLock : 1;
            ULONG fUnlock : 1;
        };
    };
};

tConfig::tConfig(int argc, TCHAR **argv)
{
    val = 0;
    parameter = 0;
    bool b = argc > 0;
    while (argc)
    {
        TCHAR *next = *argv;
        if (next[0] == '-')
        {
            switch (next[1])
            {
                case 'l': fList = 1; break;
                case 'r': fRead = 1; break;
                case 'c': fCheck = 1; break;
                case 'e': fEject = 1; break;
                case 'i': fInsert = 1; break;
                case 't': fToc = 1; break;
                case 'g': fGeometry = 1; break;
                case 's': fSeek = 1; break;
                case 'p': fParam = 1; break;
                case 'o': fOutFile = 1; break;
                case 'x': fLock = 1; break;
                case 'u': fUnlock = 1; break;
            }
        }
        else if (fParam)
        {
            fParam = 0;
            parameter = wcstoul(next, NULL, 0);
        }
        else if (fOutFile)
        {
            fOutFile = 0;
            file = next;
        }
        else
        {
            device = next;
        }
        argc--;
        argv++;
    }
}

class XFile
{
public:
    XFile(LPCWSTR name, bool bWrite, bool bDevice = false);
    XFile(ULONG number, bool bWrite);
    ~XFile() { Close(); }
    void Close()
    {
        if (m_Handle) CloseHandle(m_Handle);
        m_Handle = NULL;
    }
    bool Control(ULONG code, PVOID inBuffer, ULONG inSize, PVOID outBuffer, ULONG outSize);
    bool ControlIn(ULONG code, PVOID inBuffer, ULONG inSize)
    {
        return Control(code, inBuffer, inSize, NULL, 0);
    }
    bool ControlOut(ULONG code, PVOID outBuffer, ULONG outSize)
    {
        return Control(code, NULL, 0, outBuffer, outSize);
    }
    bool Control(ULONG code)
    {
        return Control(code, NULL, 0, NULL, 0);
    }
    bool Write(PVOID buffer, ULONG size)
    {
        return WriteFile(m_Handle, buffer, size, &m_Returned, NULL) &&
            m_Returned == size;
    }
    bool Read(PVOID buffer, ULONG size, ULONG& done)
    {
        BOOL b = ReadFile(m_Handle, buffer, size, &done, NULL);
        if (!b && !done && GetLastError() == ERROR_HANDLE_EOF)
        {
            b = true;
        }
        return !!b;
    }
    bool SeekSet(LONGLONG offset = 0)
    {
        LARGE_INTEGER li;
        li.QuadPart = offset;
        return !!SetFilePointerEx(m_Handle, li, NULL, FILE_BEGIN);
    }
    void Flush()
    {
        FlushFileBuffers(m_Handle);
    }
    ULONGLONG GetSize()
    {
        LARGE_INTEGER li;
        BOOL b = GetFileSizeEx(m_Handle, &li);
        if (!b)
        {
            Log(LOG_WARNING, "GetSize failed, error %d", GetLastError());
        }
        return b ? li.QuadPart : 0;
    }
    ULONG Returned() { return m_Returned; }
    bool IsOpen() { return m_Handle != NULL; }
protected:
    void Create(LPCWSTR name, bool bWrite);
    HANDLE m_Handle;
    ULONG m_Returned;
    bool m_bDevice;
};

class XVolumeFile : public XFile
{
public:
    XVolumeFile(LPCWSTR name, bool bWrite)
        : XFile(name, bWrite, true)
    {

    }
};

XFile::XFile(ULONG number, bool bWrite)
{
    m_bDevice = true;
    WCHAR name[32];
    swprintf_s(name, ELEMENTS_IN(name), L"\\\\.\\PhysicalDrive%d", number);
    Create(name, bWrite);
}

XFile::XFile(LPCWSTR name, bool bWrite, bool bDevice)
{
    m_bDevice = bDevice;
    m_Handle = NULL;
    Create(name, bWrite);
}

void XFile::Create(LPCWSTR name, bool bWrite)
{
    if (!name)
    {
        return;
    }
    WCHAR finalname[MAX_PATH];
    ULONG access = GENERIC_READ, share = FILE_SHARE_READ;
    if (bWrite)
    {
        access |= GENERIC_WRITE;
    }
    else
    {
        share |= FILE_SHARE_WRITE;
    }
    if (m_bDevice)
    {
        swprintf_s(finalname, ELEMENTS_IN(finalname), L"\\\\.\\%s", name);
    }
    m_Handle = CreateFile(
        m_bDevice ? finalname : name,
        access,
        share,
        NULL,
        (m_bDevice || !bWrite) ? OPEN_EXISTING : CREATE_ALWAYS,
        0,
        NULL);
    if (m_Handle == INVALID_HANDLE_VALUE)
    {
        m_Handle = NULL;
        Log(LOG_WARNING, "error: can't open %s '%S', mode %s",
            m_bDevice ? "device" : "file", name, bWrite ? "W" : "R");
    }
}

bool XFile::Control(ULONG code, PVOID inBuffer, ULONG inSize, PVOID outBuffer, ULONG outSize)
{
    if (!m_Handle)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return false;
    }
    bool b = !!DeviceIoControl(
        m_Handle,
        code,
        inBuffer,
        inSize,
        outBuffer,
        outSize,
        &m_Returned,
        NULL
    );
    if (!b)
    {
        ULONG error = GetLastError();
        ULONG fn = (code >> 2) & 0xfff;
        Log(LOG_WARNING, "error %d on fn %X", error, fn);
    }
    return b;
}

static void ListVolumes()
{
    WCHAR name[3] = L"A:";
    ULONG val = GetLogicalDrives();
    for (; val; val = val >> 1, name[0]++)
    {
        if (val & 1)
        {
            LPCSTR types[] = {"Unknown", "Wrong", "Removable", "Fixed", "Remote", "CD", "RAM"};
            UINT type = GetDriveType(name);
            Log(0, "%S - %s", name, types[type]);
        }
    }
}

static void ReadVolume(const CString& device, ULONG offset, LPCTSTR filename = NULL)
{
    XVolumeFile f(device, false);
    XFile o(filename, true);
    BYTE buffer[0x30000];
    ULONG done;
    ULONGLONG total = 0;
    ULONG tensmb = 0;
    if (offset)
    {
        bool b = f.SeekSet(offset);
        Log(0, "Seek to offset %d(0x%X) %s", offset, offset, b ? "ok" : "failed");
    }
    while (f.Read(buffer, sizeof(buffer), done) && done)
    {
        if (o.IsOpen())
        {
            o.Write(buffer, done);
        }
        total += done;
        ULONG _tensmb = (ULONG)(total / (10 * 1024 * 1024));
        if (_tensmb != tensmb)
        {
            tensmb = _tensmb;
            printf(".");
        }

    }
    Log(0, "\r\nRead done %lld", total);
}

static void CheckVolume(const CString& device)
{
    CStringA s = "Verified: ";
    XVolumeFile f(device, false);
    CDROM_DISK_DATA data;
    s += (f.Control(IOCTL_STORAGE_CHECK_VERIFY)) ? "Yes" : "No";
    if (f.ControlOut(IOCTL_CDROM_DISK_TYPE, &data, sizeof(data)))
    {
        s.AppendFormat(", type %d", data.DiskData);
    }
    GET_CONFIGURATION_IOCTL_INPUT cfgIn = { FeatureCdRead, SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL };
    GET_CONFIGURATION_HEADER cfgOut;
    if (f.Control(IOCTL_CDROM_GET_CONFIGURATION, &cfgIn, sizeof(cfgIn), &cfgOut, sizeof(cfgOut)))
    {
        s += ", CD feature OK";
    }
    Log(0, "%s", (LPCSTR)s);
}

static void EjectVolume(const CString& device)
{
    XVolumeFile f(device, false);
    f.Control(IOCTL_STORAGE_EJECT_MEDIA);
}

static void InsertVolume(const CString& device)
{
    XVolumeFile f(device, false);
    f.Control(IOCTL_STORAGE_LOAD_MEDIA);
}

#if 0
// this way of locking does not work
static void LockVolume(const CString& device, bool bLock)
{
    XFile file(filename, false);
    XVolumeFile f(device, true);

    ULONG size;
    union
    {
        CDROM_EXCLUSIVE_LOCK lock;
        CDROM_EXCLUSIVE_ACCESS unlock;
    } data;
    if (bLock)
    {
        data.lock.Access.RequestType = ExclusiveAccessLockDevice;
        data.lock.Access.Flags = CDROM_LOCK_IGNORE_VOLUME;
        CStringA s = "Yuri requested to lock it";
        memcpy_s((char *)data.lock.CallerName, sizeof(data.lock.CallerName), s.GetBuffer(), s.GetLength());
        size = sizeof(data.lock);
    }
    else
    {
        data.unlock.RequestType = ExclusiveAccessUnlockDevice;
        data.unlock.Flags = CDROM_NO_MEDIA_NOTIFICATIONS;
        size = sizeof(data.unlock);
    }
    if (f.Control(IOCTL_CDROM_EXCLUSIVE_ACCESS, &data, size, &data, size) && bLock)
    {
        MessageBox(NULL, _T("Disk is locked"), _T("Press OK to unlock the drive"), MB_OK);
        f.Close();
        LockVolume(device, false);
    }
}
#else
static void LockVolume(const CString& device, bool bLock)
{
    XVolumeFile f(device, false);
    BOOL b = bLock;

    if (f.ControlIn(IOCTL_STORAGE_MEDIA_REMOVAL, &b, sizeof(b)) && bLock)
    {
        MessageBox(NULL, _T("Disk is locked"), _T("Press OK to unlock the drive"), MB_OK);
        b = false;
        f.ControlIn(IOCTL_STORAGE_MEDIA_REMOVAL, &b, sizeof(b));
    }
}
#endif

static void QueryGeometry(const CString& device)
{
    XVolumeFile f(device, false);
    BYTE buffer[10000];
    if (f.ControlOut(IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, buffer, sizeof(buffer)))
    {
        DISK_GEOMETRY_EX *pg = (DISK_GEOMETRY_EX *)buffer;
        Log(0, "C%lldxT%dxS%d", pg->Geometry.Cylinders.QuadPart, pg->Geometry.TracksPerCylinder, pg->Geometry.SectorsPerTrack);
        Log(0, "size %lld (%llX), sector size %d, file size %lld",
            pg->DiskSize.QuadPart,
            pg->DiskSize.QuadPart,
            pg->Geometry.BytesPerSector,
            f.GetSize());
    }
}

static ULONG Frames(UCHAR *msf)
{
    ULONG res = msf[0];
    res *= 60;
    res += msf[1];
    res *= 75;
    res += msf[2];
    return res;
}

static void ReadToc(const CString& device, ULONG format)
{
    XVolumeFile f(device, false);
    union
    {
        CDROM_TOC toc;
        CDROM_TOC_SESSION_DATA tsd;
        CDROM_TOC_FULL_TOC_DATA ftd;
        ULONG buffer[0x1000];
    } toc = {};
    CDROM_READ_TOC_EX in = {};
    UINT i, n;
    switch (format)
    {
        case 0:
        case 10:
            in.Format = CDROM_READ_TOC_EX_FORMAT_TOC;
            in.Msf = format == 10;
            break;
        case 1:
        case 11:
            in.Format = CDROM_READ_TOC_EX_FORMAT_SESSION;
            in.Msf = format == 11;
            break;
        case 2:
        case 12:
            in.Format = CDROM_READ_TOC_EX_FORMAT_FULL_TOC;
            in.Msf = format == 12;
            break;
        default:
            Log(0, "Format %d not supported", format);
            return;
//            break;
    }

    if (f.Control(IOCTL_CDROM_READ_TOC, &in, sizeof(in), &toc, sizeof(toc)))
    {
        Log(0, "returned %d", f.Returned());
        switch (format)
        {
            case 10:
            case 0:
                // track data (8 bytes)
                // struct = len (2) + 2
                n = (f.Returned() - 4) / 8;
                Log(0, "Tracks %d..%d, len %02X%02X, %d tracks",
                    toc.toc.FirstTrack, toc.toc.LastTrack,
                    toc.toc.Length[0], toc.toc.Length[1],
                    n);
                for (i = 0; i < n; ++i)
                {
                    UCHAR *pc = toc.toc.TrackData[i].Address;
                    Log(0, "Tracks %d, addr %02X%02X%02X%02X (frames %d), ctl %X, adr %X",
                        toc.toc.TrackData[i].TrackNumber,
                        pc[0], pc[1], pc[2], pc[3],
                        Frames(pc + 1),
                        toc.toc.TrackData[i].Control,
                        toc.toc.TrackData[i].Adr);
                }
                break;
            case 1:
            case 11:
                n = (f.Returned() - 4) / 8;
                Log(0, "Sessions %d..%d, len %02X%02X",
                    toc.tsd.FirstCompleteSession, toc.tsd.LastCompleteSession,
                    toc.tsd.Length[0], toc.tsd.Length[1]);
                for (i = 0; i < n; ++i)
                {
                    UCHAR *pc = toc.tsd.TrackData[i].Address;
                    Log(0, "Tracks %d, addr %02X%02X%02X%02X (frames %d), ctl %X, adr %X",
                        toc.tsd.TrackData[i].TrackNumber,
                        pc[0], pc[1], pc[2], pc[3],
                        Frames(pc + 1),
                        toc.tsd.TrackData[i].Control,
                        toc.tsd.TrackData[i].Adr);
                }
                break;
            case 12:
            case 2:
                n = (f.Returned() - 4) / sizeof(toc.ftd.Descriptors[0]);
                Log(0, "Sessions %d..%d, len %02X%02X",
                    toc.ftd.FirstCompleteSession, toc.ftd.LastCompleteSession,
                    toc.ftd.Length[0], toc.ftd.Length[1]);
                for (i = 0; i < n; ++i)
                {
                    CDROM_TOC_FULL_TOC_DATA_BLOCK& db = toc.ftd.Descriptors[i];
                    Log(0, "%02d = ctl %02X, adr %d, msf %02X %02X %02X",
                        db.SessionNumber, db.Control, db.Adr,
                        db.Msf[0], db.Msf[1], db.Msf[2]);
                }
                break;
            default:
                break;
        }
    }
    else
    {
        Log(0, "error %d", GetLastError());
    }
}

int wmain(int argc, TCHAR **argv)
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else if (argc > 1)
        {
            tConfig cfg(argc - 1, argv + 1);
            Log(0, "device %S, file %S", (LPCTSTR)cfg.device, (LPCTSTR)cfg.file);
            if (cfg.fList) ListVolumes();
            if (cfg.fRead) ReadVolume(cfg.device, 0, cfg.file);
            if (cfg.fEject) EjectVolume(cfg.device);
            if (cfg.fInsert) InsertVolume(cfg.device);
            if (cfg.fCheck) CheckVolume(cfg.device);
            if (cfg.fToc) ReadToc(cfg.device, cfg.parameter);
            if (cfg.fGeometry) QueryGeometry(cfg.device);
            if (cfg.fSeek) ReadVolume(cfg.device, cfg.parameter);
            if (cfg.fLock || cfg.fUnlock) LockVolume(cfg.device, cfg.fLock);
        }
        else
        {
            puts("Common command format:");
            puts("-<command> [-p command-specific parameter] [device] [-o file]");
            puts("Supported commands:");
            puts("-l\t\t\t\tList existing volumes");
            puts("-e device\t\t\tEject device media");
            puts("-i device\t\t\tInsert device media");
            puts("-x device\t\t\tLock device media");
            puts("-u device\t\t\tUnlock device media");
            puts("-t [-p format] device\t\tRead device TOC");
            puts("\tformat: requested TOC format (0,1,2,10,11,12)");
            puts("-g device\t\t\tQuery device geometry");
            puts("-c volume\t\t\tCheck volume media");
            puts("-r device [-o file]\t\tRead to file");
            puts("\tParameter: TOC format (0,1,2,10,11,12)");
            puts("-s [-p offset] device [-o file]\tSeek and read");
            puts("\toffset - where to seek (0x)");
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
