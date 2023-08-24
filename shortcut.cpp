#include <windows.h>
#include <shobjidl.h>
#include <shlguid.h>

int main()
{
    HRESULT hres;
    IShellLink* psl = NULL;
    WCHAR szGotPath[MAX_PATH];
    WCHAR szLinkPath[] = L"C:\\Users\\user\\Desktop\\test.lnk"; // ショートカットファイルのパス

    // COM初期化
    CoInitialize(NULL);

    // IShellLinkインタフェース取得
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
        IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf = NULL;

        // IPersistFileインタフェース取得
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres))
        {
            // ショートカットの読み込み
            hres = ppf->Load(szLinkPath, STGM_READ);
            if (SUCCEEDED(hres))
            {
                // リンク先の解決
                hres = psl->Resolve(NULL, SLR_NO_UI);
                if (SUCCEEDED(hres))
                {
                    // リンク先のフルパス取得
                    hres = psl->GetPath(szGotPath, MAX_PATH, NULL, SLGP_RAWPATH);
                    if (SUCCEEDED(hres))
                    {
                        wprintf(L"リンク先：%s\n", szGotPath);
                    }
                }
            }
            ppf->Release();
        }
        psl->Release();
    }

    // COM閉じる
    CoUninitialize();

    return 0;
}
