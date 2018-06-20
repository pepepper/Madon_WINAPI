#include <windows.h>
#include <locale.h>
#include <openssl\ssl.h>
#include <openssl\err.h>
#include <openssl\bio.h>

HWND hwndset;
HANDLE  hfile;
typedef struct{
	char host[256];
	bool http;
	char token[65];
	wchar_t user[512];
}insdata;
typedef struct{
	LOGFONT lf;
	insdata insdata[10];
}settingdata;
settingdata data = {};
LPCTSTR areatext[4] = {TEXT("公開"), TEXT("未収載"), TEXT("非公開"), TEXT("ダイレクト")};
wchar_t user[512];

void getuserinfo(int num){
	int retval = 0, sum = 0, i = 0;
	BIO * bio;
	SSL *ssl;
	SSL_CTX *ctx;
	char msg[4096] = {}, *json;
	char temp[512] = {};
	char username[128] = {};
	char userid[128] = {};

	ERR_load_BIO_strings();
	SSL_load_error_strings();
	if(!data.insdata[num].http){
		OpenSSL_add_all_algorithms();
		ctx = SSL_CTX_new(SSLv23_client_method());
		bio = BIO_new_ssl_connect(ctx);
		BIO_get_ssl(bio, &ssl);
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
		BIO_set_conn_hostname(bio, data.insdata[num].host);
		BIO_ctrl(bio, BIO_C_SET_CONNECT, 1, "443");
	} else{
		bio = BIO_new_connect(data.insdata[num].host);
		BIO_ctrl(bio, BIO_C_SET_CONNECT, 1, "443");
	}
	if(BIO_do_connect(bio)<=0){
		MessageBox(NULL, TEXT("ホストに接続できませんでした"), TEXT("エラー"), MB_OK);
		BIO_free_all(bio);
		if(!data.insdata[num].http)SSL_CTX_free(ctx);
		return;
	}

	sprintf(msg, u8"GET /api/v1/accounts/verify_credentials HTTP/1.1\r\nHost: %s\r\nAuthorization: Bearer %s\r\n\r\n", data.insdata[num].host, data.insdata[num].token);
	BIO_write(bio, msg, strlen(msg));
	retval = BIO_read(bio, msg+retval, 4096-retval);
	sum = sum+retval;
	retval = BIO_read(bio, msg+retval, 4096-retval);
	sum = sum+retval;
	BIO_shutdown_wr(bio);
	BIO_free_all(bio);
	if(!data.insdata[num].http)SSL_CTX_free(ctx);
	msg[12] = 0;
	if(strcmp(msg, "HTTP/1.1 200")){
		return;
	}
	while(msg[sum-i]!='}'){
		msg[sum-i] = 0;
		i++;
	}
	json = msg+635;

	sscanf(json, u8"{\"id\":\"%*d\",\"username\":\"%[^\"]\",\"acct\":\"%*[^\"]\",\"display_name\":\"%[^\"]\"%*s", userid, username);//json parse ///

	sprintf(temp, u8"%s|%s@%s", username, userid, data.insdata[num].host);
	MultiByteToWideChar(CP_UTF8, 0, temp, -1, user, 512);
}

int sendtoot(int area, char *bodytext, int cw, char *cwtext, int num){
	int retval = 0;
	BIO * bio;
	SSL *ssl;
	SSL_CTX *ctx;
	char msg[4096] = {}, msg1[2048] = {}, boundary[33] = {};
	char areatext[32];
	wchar_t dialog[2048];
	switch(area){
		case 0:
			strcpy(areatext, "public");
			break;
		case 1:
			strcpy(areatext, "unlisted");
			break;
		case 2:
			strcpy(areatext, "private");
			break;
		case 3:
			strcpy(areatext, "direct");
			break;
	}
	for(int i = 0;i<4;i++){
		sprintf(boundary, u8"%s%c%c%c%c%c%c%c%c", boundary, u8'A'+(char)(rand()%26), u8'a'+(char)(rand()%26), u8'0'+(char)(rand()%10), u8'a'+(char)(rand()%26), u8'A'+(char)(rand()%26), u8'0'+(char)(rand()%10), u8'0'+(char)(rand()%10), u8'A'+(char)(rand()%26));
	}
	if(!cw){
		sprintf(msg1, u8"--------%s\r\nContent-Disposition: form-data; name=\"status\"\r\n\r\n%s\r\n--------%s\r\nContent-Disposition: form-data; name=\"visibility\"\r\n\r\n%s\r\n--------%s--", boundary, bodytext, boundary, areatext, boundary);
		sprintf(msg, u8"POST /api/v1/statuses HTTP/1.1\r\nHost: %s\r\nAuthorization: Bearer %s\r\nAccept: */*\r\nExpect: 100-continue\r\nContent-Type: multipart/form-data; boundary=------%s\r\nContent-Length: %d\r\n\r\n", data.insdata[num].host, data.insdata[num].token, boundary, strlen(msg1));
	} else{
		sprintf(msg1, u8"--------%s\r\nContent-Disposition: form-data; name=\"status\"\r\n\r\n%s\r\n--------%s\r\nContent-Disposition: form-data; name=\"visibility\"\r\n\r\n%s\r\n--------%s\r\nContent-Disposition: form-data; name=\"spoiler_text\"\r\n\r\n%s\r\n--------%s--", boundary, bodytext, boundary, areatext, boundary, cwtext, boundary);
		sprintf(msg, u8"POST /api/v1/statuses HTTP/1.1\r\nHost: %s\r\nAuthorization: Bearer %s\r\nAccept: */*\r\nExpect: 100-continue\r\nContent-Type: multipart/form-data; boundary=------%s\r\nContent-Length: %d\r\n\r\n", data.insdata[num].host, data.insdata[num].token, boundary, strlen(msg1));
	}
	ERR_load_BIO_strings();
	SSL_load_error_strings();
	if(!data.insdata[num].http){
		OpenSSL_add_all_algorithms();
		ctx = SSL_CTX_new(SSLv23_client_method());
		bio = BIO_new_ssl_connect(ctx);
		BIO_get_ssl(bio, &ssl);
		SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
		BIO_set_conn_hostname(bio, data.insdata[num].host);
		BIO_ctrl(bio, BIO_C_SET_CONNECT, 1, "443");

	} else{
		bio = BIO_new_connect(data.insdata[num].host);
		BIO_ctrl(bio, BIO_C_SET_CONNECT, 1, "443");
	}

	if(BIO_do_connect(bio)<=0){
		MessageBox(NULL, TEXT("ホストに接続できませんでした"), TEXT("エラー"), MB_OK);
		BIO_free_all(bio);
		if(!data.insdata[num].http)SSL_CTX_free(ctx);
		return 0;
	}
	BIO_write(bio, msg, strlen(msg));
	BIO_flush(bio);
	BIO_write(bio, msg1, strlen(msg1));
	BIO_flush(bio);
	//	/*
	for(;;){
		retval = BIO_read(bio, msg, 2048);
		if(retval<=0)break;
		msg[retval] = 0;
		MultiByteToWideChar(CP_UTF8, 0, msg, -1, dialog, 2048);
		if(strcmp(msg, "0"))break;
	}
	//	*/
	BIO_shutdown_wr(bio);
	BIO_free_all(bio);
	if(!data.insdata[num].http)SSL_CTX_free(ctx);
	return 0;
}

LRESULT CALLBACK SetWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
	static HWND edithost, edittoken, check, userlist;
	static CHOOSEFONT cf;
	wchar_t temp[2];
	DWORD readsize;
	LPTSTR lhostname, ltoken;
	static int select;
	switch(msg){
		case WM_CREATE:
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner = hwnd;
			cf.lpLogFont = &(data.lf);
			cf.Flags = CF_EFFECTS|CF_SCREENFONTS;

			userlist = CreateWindow(TEXT("COMBOBOX"), NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|CBS_DROPDOWNLIST|WS_VSCROLL, 0, 0, 300, 200, hwnd, (HMENU)5, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			for(int i = 0; i<10; i++){
				wsprintf(temp, TEXT("%d"), i);
				SendMessage(userlist, CB_ADDSTRING, 0, (LPARAM)temp);
			}
			SendMessage(userlist, CB_SETCURSEL, 0, 0);
			select = 0;
			edithost = CreateWindow(TEXT("EDIT"), TEXT(""), WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT, 0, 35, 800, 25, hwnd, (HMENU)1, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			lhostname = (LPTSTR)malloc(sizeof(wchar_t)*(strlen(data.insdata[0].host)+2));
			mbstowcs(lhostname, data.insdata[0].host, 2048);
			SetWindowText(edithost, lhostname);
			free(lhostname);

			edittoken = CreateWindow(TEXT("EDIT"), TEXT(""), WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT, 0, 70, 800, 25, hwnd, (HMENU)2, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			ltoken = (LPTSTR)malloc(sizeof(wchar_t)*(strlen(data.insdata[0].token)+2));
			mbstowcs(ltoken, data.insdata[0].token, 128);
			SetWindowText(edittoken, ltoken);
			free(ltoken);

			CreateWindow(TEXT("BUTTON"), TEXT("フォント変更"), WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, 0, 105, 100, 50, hwnd, (HMENU)3, ((LPCREATESTRUCT)(lp))->hInstance, NULL);

			check = CreateWindow(TEXT("BUTTON"), TEXT("HTTP Only"), WS_CHILD|WS_VISIBLE|BS_CHECKBOX|BS_LEFTTEXT, 0, 165, 50, 20, hwnd, (HMENU)4, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(check, BM_SETCHECK, data.insdata[0].http, 0);

			break;
		case WM_CLOSE:
			lhostname = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(edithost)+2));
			GetWindowText(edithost, lhostname, 256);
			wcstombs(data.insdata[select].host, lhostname, 256);
			free(lhostname);
			ltoken = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(edittoken)+2));
			GetWindowText(edittoken, ltoken, 65);
			wcstombs(data.insdata[select].token, ltoken, 65);
			free(ltoken);

			SetFilePointer(hfile, 0, NULL, FILE_BEGIN);
			WriteFile(hfile, &data, sizeof(data), &readsize, NULL);
			if(sizeof(data)!=readsize)MessageBox(hwnd, TEXT("設定ファイルの保存に失敗しました\nもう一度設定ダイアログを開いてから閉じてください"), TEXT("エラー"), MB_OK);
			FlushFileBuffers(hfile);
			ShowWindow(hwnd, FALSE);
			return 0;
		case WM_COMMAND:
			switch(LOWORD(wp)){
				case 5:
					if(HIWORD(wp)==CBN_SELCHANGE){
						lhostname = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(edithost)+2));
						GetWindowText(edithost, lhostname, 256);
						wcstombs(data.insdata[select].host, lhostname, 256);
						free(lhostname);
						ltoken = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(edittoken)+2));
						GetWindowText(edittoken, ltoken, 65);
						wcstombs(data.insdata[select].token, ltoken, 65);
						free(ltoken);

						select = SendMessage(userlist, CB_GETCURSEL, 0, 0);
						lhostname = (LPTSTR)malloc(sizeof(wchar_t)*(strlen(data.insdata[select].host)+2));
						mbstowcs(lhostname, data.insdata[select].host, 256);
						SetWindowText(edithost, lhostname);
						free(lhostname);
						ltoken = (LPTSTR)malloc(sizeof(wchar_t)*(strlen(data.insdata[select].token)+2));
						mbstowcs(ltoken, data.insdata[select].token, 65);
						SetWindowText(edittoken, ltoken);
						free(ltoken);
					}
					break;
				case 4:
					if(SendMessage(check, BM_GETCHECK, 0, 0)){
						SendMessage(check, BM_SETCHECK, false, 0);
						data.insdata[SendMessage(userlist, CB_GETCURSEL, 0, 0)].http = false;
					} else{
						SendMessage(check, BM_SETCHECK, true, 0);
						data.insdata[SendMessage(userlist, CB_GETCURSEL, 0, 0)].http = true;
					}
					break;
				case 3:
					if(!ChooseFont(&cf)) return 0;
			}

	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
	static HWND check, cwedit, bodyedit, arealist, tootbutton, userlist, refuser, media;
	static OPENFILENAME ofn = {0};
	HFONT hFont;
	static HMENU menu, menuitem;
	LPTSTR temp;
	MENUITEMINFO mii;
	DWORD readsize;
	LPTSTR sendbody, sendcw;
	char *csendbody, *csendcw;
	int limitref[10] = {}, select;

	switch(msg){
		case WM_TIMER:
			KillTimer(hwnd, wp);
			if(SendMessage(userlist, CB_GETCURSEL, 0, 0)==wp)EnableWindow(refuser, TRUE);
			limitref[wp] = 0;
			return 0;
		case WM_DESTROY:
			CloseHandle(hfile);
			PostQuitMessage(0);
			return 0;
		case WM_CREATE:
			hfile = CreateFile(TEXT(".\\settings.dat"), GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if(hfile==INVALID_HANDLE_VALUE)MessageBox(hwnd, TEXT("設定ファイルを開けません\n今回のプロセスで設定の読み出し・保存は行われません"), TEXT("エラー"), MB_OK);
			if(GetFileSize(hfile, NULL)){
				ReadFile(hfile, &data, sizeof(settingdata), &readsize, NULL);
				if(sizeof(settingdata)!=readsize){
					MessageBox(hwnd, TEXT("設定ファイルが壊れています\n初期設定で起動します"), TEXT("エラー"), MB_OK);
					data.lf.lfHeight = 0;
					data.lf.lfWidth = data.lf.lfEscapement = data.lf.lfOrientation = 0;
					data.lf.lfWeight = FW_NORMAL;
					data.lf.lfItalic = data.lf.lfUnderline = FALSE;
					data.lf.lfStrikeOut = FALSE;
					data.lf.lfCharSet = SHIFTJIS_CHARSET;
					data.lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
					data.lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
					data.lf.lfQuality = DEFAULT_QUALITY;
					data.lf.lfPitchAndFamily = 0;
					data.lf.lfFaceName[0] = '\0';
				}
			}

			hFont = CreateFontIndirect(&(data.lf));

			mii.cbSize = sizeof(MENUITEMINFO);
			mii.fMask = MIIM_TYPE|MIIM_ID;
			mii.fType = MFT_STRING;
			mii.dwTypeData = TEXT("設定");
			mii.wID = 6;
			menu = CreateMenu();
			InsertMenuItem(menu, 6, FALSE, &mii);
			SetMenu(hwnd, menu);

			userlist = CreateWindow(TEXT("COMBOBOX"), NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|CBS_DROPDOWNLIST|WS_VSCROLL, 0, 0, 240, 200, hwnd, (HMENU)7, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			//for(int i = 0; i<4; i++)	
			for(int i = 0;i<10;i++){
				if(data.insdata[i].host[0]==0)break;
				getuserinfo(i);
				SendMessage(userlist, CB_ADDSTRING, 0, (LPARAM)user);
			}
			SendMessage(userlist, CB_SETCURSEL, 0, 0);
			SendMessage(userlist, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			refuser = CreateWindow(TEXT("BUTTON"), TEXT("更新"), WS_CHILD|WS_VISIBLE, 250, 0, 50, 25, hwnd, (HMENU)4, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(refuser, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			check = CreateWindow(TEXT("BUTTON"), TEXT("CW"), WS_CHILD|WS_VISIBLE|BS_CHECKBOX|BS_LEFTTEXT, 0, 35, 50, 20, hwnd, (HMENU)1, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(check, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			media = CreateWindow(TEXT("BUTTON"), TEXT("メディア"), WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, 60, 35, 60, 20, hwnd, (HMENU)8, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(media, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			cwedit = CreateWindow(TEXT("EDIT"), TEXT("ここに警告を書いてください"), WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|WS_BORDER|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_LEFT|ES_MULTILINE, 0, 65, 300, 100, hwnd, (HMENU)2, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(cwedit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
			EnableWindow(cwedit, FALSE);

			bodyedit = CreateWindow(TEXT("EDIT"), TEXT("今なにしてる?"), WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|WS_BORDER|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_LEFT|ES_MULTILINE, 0, 175, 300, 300, hwnd, (HMENU)3, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(bodyedit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			arealist = CreateWindow(TEXT("COMBOBOX"), NULL, WS_CHILD|WS_VISIBLE|WS_BORDER|CBS_DROPDOWNLIST|WS_VSCROLL, 0, 485, 100, 200, hwnd, NULL, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			for(int i = 0; i<4; i++)	SendMessage(arealist, CB_ADDSTRING, 0, (LPARAM)areatext[i]);
			SendMessage(arealist, CB_SETCURSEL, 0, 0);
			SendMessage(arealist, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			tootbutton = CreateWindow(TEXT("BUTTON"), TEXT("トゥート!"), WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON, 0, 525, 100, 50, hwnd, (HMENU)5, ((LPCREATESTRUCT)(lp))->hInstance, NULL);
			SendMessage(tootbutton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));

			return 0;
		case WM_COMMAND:
			switch(LOWORD(wp)){
				case 1:
					if(SendMessage(check, BM_GETCHECK, 0, 0)){
						SendMessage(check, BM_SETCHECK, false, 0);
						EnableWindow(cwedit, FALSE);
					} else{
						SendMessage(check, BM_SETCHECK, true, 0);
						EnableWindow(cwedit, TRUE);
					}
					break;
				case 2:
					switch(HIWORD(wp)){
						case EN_SETFOCUS:
							temp = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(cwedit)+2));
							GetWindowText(cwedit, temp, GetWindowTextLength(cwedit)+sizeof(wchar_t)*2);
							if(!lstrcmp(temp, TEXT("ここに警告を書いてください")))SetWindowText(cwedit, TEXT(""));
							free(temp);
							break;
						case EN_KILLFOCUS:
							temp = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(cwedit)+2));
							GetWindowText(cwedit, temp, GetWindowTextLength(cwedit)+2);
							if(!lstrcmp(temp, TEXT("")))SetWindowText(cwedit, TEXT("ここに警告を書いてください"));
							free(temp);
							break;
					}
					break;
				case 3:
					switch(HIWORD(wp)){
						case EN_SETFOCUS:
							temp = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(bodyedit)+2));
							GetWindowText(bodyedit, temp, GetWindowTextLength(bodyedit)+sizeof(wchar_t)*2);
							if(!lstrcmp(temp, TEXT("今なにしてる?")))SetWindowText(bodyedit, TEXT(""));
							free(temp);
							break;
						case EN_KILLFOCUS:
							temp = (LPTSTR)malloc(sizeof(wchar_t)*(GetWindowTextLength(bodyedit)+2));
							GetWindowText(bodyedit, temp, GetWindowTextLength(bodyedit)+2);
							if(!lstrcmp(temp, TEXT("")))SetWindowText(bodyedit, TEXT("今なにしてる?"));
							free(temp);
							break;
					}
					break;
				case 4:
					select = SendMessage(userlist, CB_GETCURSEL, 0, 0);
					if(select>=0){
						getuserinfo(select);
						SendMessage(userlist, CB_DELETESTRING, select, 0);
						SendMessage(userlist, CB_INSERTSTRING, select, (LPARAM)user);
						SendMessage(userlist, CB_SETCURSEL, select, 0);
						limitref[select] = 1;
						EnableWindow(refuser, FALSE);
						SetTimer(hwnd, (UINT)select, 120000, NULL);
					}
					break;
				case 5:
					sendbody = (LPTSTR)malloc(sizeof(wchar_t)*502);
					GetWindowText(bodyedit, sendbody, 500);
					csendbody = (char*)malloc(sizeof(char)*502);
					WideCharToMultiByte(CP_UTF8, 0, sendbody, -1, csendbody, 502, NULL, NULL);
					free(sendbody);
					sendbody = NULL;
					sendcw = (LPTSTR)malloc(sizeof(wchar_t)*502);
					GetWindowText(cwedit, sendcw, 500);
					csendcw = (char*)malloc(sizeof(char)*502);
					WideCharToMultiByte(CP_UTF8, 0, sendcw, -1, csendcw, 502, NULL, NULL);
					free(sendcw);
					sendbody = NULL;
					if((!strcmp(csendcw, u8"ここに警告を書いてください")&&SendMessage(check, BM_GETCHECK, 0, 0))||!strcmp(csendbody, u8"今なにしてる?")){
						if(MessageBox(hwnd, TEXT("テキストが初期状態のままです\nこのまま送信しますか?"), TEXT("警告"), MB_OKCANCEL)==IDCANCEL)return 0;
					}
					sendtoot(SendMessage(arealist, CB_GETCURSEL, 0, 0), csendbody, SendMessage(check, BM_GETCHECK, 0, 0), csendcw, SendMessage(userlist, CB_GETCURSEL, 0, 0));
					free(csendbody);
					free(csendcw);
					SetWindowText(bodyedit, TEXT("今なにしてる?"));
					SetWindowText(cwedit, TEXT("ここに警告を書いてください"));
					break;
				case 6:
					ShowWindow(hwndset, TRUE);
					return 0;
				case 7:
					if(HIWORD(wp)==CBN_SELCHANGE){
						if(!limitref[SendMessage(userlist, CB_GETCURSEL, 0, 0)])EnableWindow(refuser, TRUE);
					}
					break;
				case 8:
					break;
			}
			return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow){
	HWND hwnd;
	MSG msg;
	WNDCLASS winc;
	winc.style = CS_HREDRAW|CS_VREDRAW;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("Main");

	if(!RegisterClass(&winc)) return -1;

	hwnd = CreateWindow(TEXT("Main"), TEXT("Madon"), WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if(hwnd==NULL) return -1;

	winc.lpfnWndProc = SetWndProc;
	winc.lpszClassName = TEXT("Setting");
	RegisterClass(&winc);
	hwndset = CreateWindow(TEXT("Setting"), TEXT("設定"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	while(GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
