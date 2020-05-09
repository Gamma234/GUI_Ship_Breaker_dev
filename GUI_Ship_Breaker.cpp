#include <windows.h>
#include <iostream> 
#include "Gamma.h"
#include "GameSystem.h"

using namespace std ;

HWND consoleWnd ;

SBG::GameSystem * game ;
vector<HWND> buttons ;
HWND hMainWindow, hTitleWindow, hWarningWindow, hShowInfoButton ;
string warningStr ;
bool showInfo ;

void RegisterDialogClass( HINSTANCE hInst ) ;
void AddControls( HWND hwnd ) ;

HWND hPlayDlg, conWindow ;

void DisplayPlayWindow( HWND hwnd ) ;
LRESULT CALLBACK PlayDialogProcedure( HWND hwnd, UINT msg, WPARAM wP, LPARAM lp ) ;
void ShowShipInfoOnButtons() ;
void ScrollSizeSetting( HWND hwnd, SCROLLINFO &si ) ;
void ScrollVSetting( HWND hwnd, WPARAM wParam, SCROLLINFO &si ) ;
void ScrollHSetting( HWND hwnd, WPARAM wParam, SCROLLINFO &si ) ;

HWND hSettingDlg, shipButton, mapButton ;
HWND shipNumTextWnd, nameTextWnd, sizeTextWnd, mvTextWnd, bsTextWnd, messageWnd, setShipNumberButton, 
     shipNumCondition, nameCondition, sizeCondition, mvCondition, bsCondition, setShipButton ;
int totalShip, existShip ;
HWND xTextWnd, yTextWnd, xCondition, yCondition ;

void DisplaySettingWindow( HWND hwnd ) ;	
LRESULT CALLBACK SettingDialogProcedure( HWND hwnd, UINT msg, WPARAM wP, LPARAM lp ) ;
void DisplaySetShip() ;
void CheckShipNumberSetting() ;
void CheckShipSetting() ;
void DisplaySetMap() ;
void CheckMapSetting( int min, int max ) ; // [min,max]

#define PLAY_BUTTON 0
#define SETTING_BUTTON 1
#define SHOW_INFO 2 
#define QUIT 1000

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	
	switch(Message) {
		
		case WM_CREATE : {
			game = new SBG::GameSystem( 5, 5, 3, &warningStr ) ;
			AddControls( hwnd ) ;
			break;
		} // WM_CREATE
			
		case WM_COMMAND : {
			
			switch ( wParam ) {				
				case PLAY_BUTTON :
					DisplayPlayWindow( hwnd ) ;	
					break ;					
				case SETTING_BUTTON :
					DisplaySettingWindow( hwnd ) ;	
					break ;				
				case SHOW_INFO :
					showInfo = ( showInfo ? false : true ) ;
					SetWindowText( hShowInfoButton, ( showInfo ? "ON" : "OFF" ) ) ;
					break ;				
				case QUIT :
					DestroyWindow( hwnd ) ;
					break ;				
			} // switch 
			
			break;
			
		} // WM_COMMAND
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			delete game ;
			PostQuitMessage(0);
			ShowWindow( consoleWnd, SW_SHOW ) ; 
			break;
		} // WM_DESTROY
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	
	return 0;
	
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	WNDCLASSEX wc; /* A properties struct of our window */
	MSG msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	RegisterDialogClass( hInstance ) ;

	hMainWindow = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Ship_Breaker",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		10, /* x */ // CW_USEDEFAULT
		10, /* y */ // CW_USEDEFAULT
		640, /* width */
		500, /* height */
		NULL,NULL,hInstance,NULL);

	if( hMainWindow == NULL ) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	} // if  
	
	consoleWnd = GetConsoleWindow() ; 
	ShowWindow( consoleWnd, SW_HIDE ) ; 

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	
	return msg.wParam;
	
} // int WINAPI WinMain()

void RegisterDialogClass( HINSTANCE hInst ) {
	
	WNDCLASSW playDialog = {0} ;
	
	playDialog.hbrBackground = (HBRUSH)COLOR_WINDOW ;
	playDialog.hCursor = LoadCursor( NULL, IDC_CROSS ) ;
	playDialog.hInstance = hInst ;
	playDialog.lpszClassName = L"myPlayDialogClass" ;
	playDialog.lpfnWndProc = PlayDialogProcedure ;
	
	RegisterClassW( &playDialog ) ;
	
	WNDCLASSW settingDialog = {0} ;
	
	settingDialog.hbrBackground = (HBRUSH)COLOR_WINDOW ;
	settingDialog.hCursor = LoadCursor( NULL, IDC_CROSS ) ;
	settingDialog.hInstance = hInst ;
	settingDialog.lpszClassName = L"mySettingDialogClass" ;
	settingDialog.lpfnWndProc = SettingDialogProcedure ;
	
	RegisterClassW( &settingDialog ) ;
	
} // RegisterDialogClass()

void AddControls( HWND hwnd ) {
	
	showInfo = false ;
	
	hTitleWindow = CreateWindow( "Edit", game->Introduction().c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
								 WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY, 100, 50, 500, 150,
								 hwnd, NULL, NULL, NULL ) ;
			
	CreateWindow( "Button", "Play", WS_VISIBLE | WS_CHILD, 100, 230, 100, 40, hwnd, (HMENU)PLAY_BUTTON, NULL, NULL ) ;
	
	CreateWindow( "Static", "Show Info :", WS_VISIBLE | WS_CHILD, 80, 280, 77, 20, hwnd, NULL, NULL, NULL ) ;
	hShowInfoButton = CreateWindow( "Button", "OFF", WS_VISIBLE | WS_CHILD, 160, 280, 40, 20, hwnd, (HMENU)SHOW_INFO, NULL, NULL ) ;
					 
	CreateWindow( "Button", "Setting", WS_VISIBLE | WS_CHILD, 100, 330, 100, 40, hwnd, (HMENU)SETTING_BUTTON, NULL, NULL ) ;
	
	CreateWindow( "Button", "Quit", WS_VISIBLE | WS_CHILD, 100, 380, 100, 40, hwnd, (HMENU)QUIT, NULL, NULL ) ;
	
	hWarningWindow = CreateWindow( "Edit", warningStr.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
								 WS_VSCROLL | WS_HSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_READONLY, 250, 230, 300, 200,
								 hwnd, NULL, NULL, NULL ) ;
						
} // AddControls()

//--------------------------------------------------------------------------------------------------

#define QUIT_PLAY 10000
#define SCROLL_V_MAX 100 
#define SCROLL_V_BAR 65 
#define SCROLL_H_MAX 100 
#define SCROLL_H_BAR 50

void DisplayPlayWindow( HWND hWnd ) {
	
	buttons.clear() ;
	
	hPlayDlg = CreateWindowW( L"myPlayDialogClass", L"Game", WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL ,
							20, 20, 500, 600, hWnd, NULL, NULL, NULL ) ;
	
	int i = 0, j = 0, ch = 97, loc = 0, vertical = 0, horizon = 0 ;
	
	for ( i = 0, vertical = 105 ; i < game->GetMapX() ; i++, vertical += 30, ch++ ) {
		string str = "" ;
		str = str + (char)ch ;
		CreateWindow( "Static", str.c_str(), WS_VISIBLE | WS_CHILD,
					     vertical, 50, 25, 25, hPlayDlg, NULL, NULL, NULL ) ;
	} // for
	
	for ( i = 0, horizon = 72 ; i < game->GetMapY() ; i++, horizon += 30 ) {
		CreateWindow( "Static", GA::IntToString( i ).c_str(), WS_VISIBLE | WS_CHILD,
					 	 77, horizon, 25, 25, hPlayDlg, NULL, NULL, NULL ) ;
	} // for 
	
	for ( i = 0, vertical = 70, loc = 0 ; i < game->GetMapY() ; i++, vertical += 30 ) {
    	for ( j = 0, horizon = 98 ; j < game->GetMapX() ; j++, horizon += 30, loc++ ) {
    		buttons.push_back( CreateWindowW( L"Button", L"", WS_VISIBLE | WS_CHILD,
							 					horizon, vertical, 25, 25, hPlayDlg, (HMENU)loc, NULL, NULL ) ) ;
		} // for
	} // for 
	
	CreateWindow( "Button", "Quit", WS_VISIBLE | WS_CHILD, 75, 12, 50, 25, hPlayDlg, (HMENU)QUIT_PLAY, NULL, NULL ) ;
	
	conWindow = CreateWindow( "Static", "hit condition", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 12, 500, 25, hPlayDlg, NULL, NULL, NULL ) ;
	
	if ( showInfo ) ShowShipInfoOnButtons() ;
	
	if ( game->AvailableShipNum() == 0 ) {
		MessageBeep( MB_ICONINFORMATION ) ;	
		MessageBox( hMainWindow, "Ship Number == 0", "Lack Of Ship", MB_OK ) ;
		DestroyWindow( hPlayDlg ) ;
		return ;
	} // if
	
	EnableWindow( hWnd, false ) ;
	
} // DisplayPlayWindow()

LRESULT CALLBACK PlayDialogProcedure( HWND hwnd, UINT msg, WPARAM wP, LPARAM lp ) {
	
	SCROLLINFO si ; 
	
	switch ( msg ) {
		
		case WM_COMMAND : {
			
			if ( wP == QUIT_PLAY ) {
				EnableWindow( hPlayDlg, false ) ;
				MessageBeep( MB_ICONINFORMATION ) ;	
				MessageBox( hMainWindow, "Quit !", "END", MB_OK ) ;
				DestroyWindow( hwnd ) ;
			} // if
			else {
				
				string conditionStr = game->Play( wP ) ;
				SetWindowText( conWindow, conditionStr.c_str() ) ;
				
				if ( conditionStr == "Successful !" ) {
					EnableWindow( hPlayDlg, false ) ;
					MessageBeep( MB_ICONINFORMATION ) ;	
					MessageBox( hMainWindow, conditionStr.c_str(), "END", MB_OK ) ;
					DestroyWindow( hwnd ) ;
				} // if
				
				if ( showInfo ) ShowShipInfoOnButtons() ;
				
			} // else
			
			break;
			
		} // WM_COMMAND 
		
		case WM_SIZE :
			ScrollSizeSetting( hwnd, si ) ;
			break ;
		
		case WM_VSCROLL :
			ScrollVSetting( hwnd, wP, si ) ;
			break ;
		
		case WM_HSCROLL :
			ScrollHSetting( hwnd, wP, si ) ;
			break ;
		
		case WM_CLOSE :
			EnableWindow( hMainWindow, true ) ;
			DestroyWindow( hwnd ) ;
			break ;
			
		case WM_DESTROY :
			EnableWindow( hMainWindow, true ) ;
			game->SetGame() ;
			SetWindowText( hTitleWindow, game->Introduction().c_str() ) ;	
			SetWindowText( hWarningWindow, warningStr.c_str() ) ;
			break ;
			
		default :
			return DefWindowProcW( hwnd, msg, wP, lp ) ;
		
	} // switch	
	
} // PlayDialogProcedure()

void ShowShipInfoOnButtons() {
	
	for ( int i = 0 ; i < buttons.size() ; i++ ) {
		SetWindowText( buttons[i], game->GetLocationInfo( i ).c_str() ) ;
	} // for 
	
} // ShowShipInfoOnButtons() 

void ScrollSizeSetting( HWND hwnd, SCROLLINFO &si ) {
	
	// Set the vertical scrolling range and page size 
	si.cbSize = sizeof(si) ; 
    si.fMask  = SIF_RANGE | SIF_PAGE ; 
    si.nMin   = 0 ; 
    si.nMax   = SCROLL_V_MAX - 1 ; 
    si.nPage  = SCROLL_V_BAR ; 
    SetScrollInfo( hwnd, SB_VERT, &si, TRUE ) ; 

	// Set the horizontal scrolling range and page size  
    si.cbSize = sizeof(si) ; 
    si.fMask  = SIF_RANGE | SIF_PAGE ; 
    si.nMin   = 0 ; 
    si.nMax   = SCROLL_H_MAX - 1 ; 
    si.nPage  = SCROLL_H_BAR ; 
    SetScrollInfo( hwnd, SB_HORZ, &si, TRUE ) ; 
	
} // ScrollSizeSetting()

void ScrollVSetting( HWND hwnd, WPARAM wParam, SCROLLINFO &si ) {
	
	// Get all the vertial scroll bar information.
    si.cbSize = sizeof ( si ) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo ( hwnd, SB_VERT, &si ) ;

    // Save the position for comparison later on.
    int yPos = si.nPos;
    
    switch ( LOWORD(wParam) ) {

	    // User clicked the HOME keyboard key.
	    case SB_TOP :
	        si.nPos = si.nMin ;
	        break ;
	          
	    // User clicked the END keyboard key.
	    case SB_BOTTOM :
	        si.nPos = si.nMax ;
	        break ;
	          
	    // User clicked the top arrow.
	    case SB_LINEUP :
	        si.nPos -= 1 ;
	        break ;
	          
	    // User clicked the bottom arrow.
	    case SB_LINEDOWN :
	        si.nPos += 1 ;
	        break ;
	          
	    // User clicked the scroll bar shaft above the scroll box.
	    case SB_PAGEUP :
	        si.nPos -= si.nPage ;
	        break ;
	          
	    // User clicked the scroll bar shaft below the scroll box.
	    case SB_PAGEDOWN :
	        si.nPos += si.nPage ;
	        break ;
	          
	    // User dragged the scroll box.
	    case SB_THUMBTRACK :
	        si.nPos = si.nTrackPos ;
	        break ;
	          
	    default:
	        break ; 
    } // switch

    // Set the position and then retrieve it.  Due to adjustments
    // by Windows it may not be the same as the value set.
    si.fMask = SIF_POS;
    SetScrollInfo ( hwnd, SB_VERT, &si, TRUE ) ;
    GetScrollInfo ( hwnd, SB_VERT, &si ) ;

    // If the position has changed, scroll window and update it.
    if ( si.nPos != yPos ) {                    
        ScrollWindow( hwnd, 0, 10 * ( yPos - si.nPos ), NULL, NULL ) ;
        //UpdateWindow( hwnd ) ;
    } // if
	
} // ScrollVSetting()

void ScrollHSetting( HWND hwnd, WPARAM wParam, SCROLLINFO &si ) {
	
	// Get all the vertial scroll bar information.
    si.cbSize = sizeof( si ) ;
    si.fMask  = SIF_ALL ;
    GetScrollInfo( hwnd, SB_HORZ, &si ) ;

    // Save the position for comparison later on.     
    int xPos = si.nPos;
    
    switch( LOWORD(wParam) ) {
    	
	    // User clicked the left arrow.
	    case SB_LINELEFT : 
	        si.nPos -= 1 ;
	        break ;
	          
	    // User clicked the right arrow.
	    case SB_LINERIGHT : 
	        si.nPos += 1 ;
	        break ;
	          
	    // User clicked the scroll bar shaft left of the scroll box.
	    case SB_PAGELEFT :
	        si.nPos -= si.nPage ;
	        break ;
	          
	    // User clicked the scroll bar shaft right of the scroll box.
	    case SB_PAGERIGHT :
	        si.nPos += si.nPage ;
	        break ;
	          
	    // User dragged the scroll box.
	    case SB_THUMBTRACK : 
	        si.nPos = si.nTrackPos ;
	        break ;
	          
	    default :
	        break ;
    }

    // Set the position and then retrieve it.  Due to adjustments
    // by Windows it may not be the same as the value set.
    si.fMask = SIF_POS ;
    SetScrollInfo ( hwnd, SB_HORZ, &si, TRUE ) ;
    GetScrollInfo ( hwnd, SB_HORZ, &si ) ;
     
    // If the position has changed, scroll the window.
    if ( si.nPos != xPos ) {
        ScrollWindow( hwnd, 10 * (xPos - si.nPos), 0, NULL, NULL );
    } // if
	
} // ScrollHSetting()

//------------------------------------------------------------------------------------------------------

#define SHIP_BUTTON 1
#define MAP_BUTTON 2
#define SET_SHIP_NUMBER_OK 3
#define SET_SHIP_OK 4
#define SET_MAP_OK 5
#define QUIT_SETTING 1000

void DisplaySettingWindow( HWND hwnd ) {
	
	hSettingDlg = CreateWindowW( L"mySettingDialogClass", L"Setting", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 20, 20, 600, 300, hwnd, NULL, NULL, NULL ) ;
	
	shipButton = CreateWindow( "Button", "set ship", WS_VISIBLE | WS_CHILD, 75, 50, 100, 50, hSettingDlg, (HMENU)SHIP_BUTTON, NULL, NULL ) ;
	
	mapButton = CreateWindow( "Button", "set map", WS_VISIBLE | WS_CHILD, 75, 125, 100, 50, hSettingDlg, (HMENU)MAP_BUTTON, NULL, NULL ) ;
	
	CreateWindow( "Button", "Quit", WS_VISIBLE | WS_CHILD, 75, 200, 50, 30, hSettingDlg, (HMENU)QUIT_SETTING, NULL, NULL ) ;
	
	EnableWindow( hwnd, false ) ;
	
} // DisplaySettingWindow()

LRESULT CALLBACK SettingDialogProcedure( HWND hwnd, UINT msg, WPARAM wP, LPARAM lp ) {
	
	switch ( msg ) {
		
		case WM_COMMAND : {
			
			switch ( wP ) {
				
				case QUIT_SETTING :
					EnableWindow( hMainWindow, true ) ; 
					DestroyWindow( hwnd ) ;
					break ;
				
				case SHIP_BUTTON :
					EnableWindow( shipButton, false ) ;
					EnableWindow( mapButton, false ) ;
					DisplaySetShip() ;
					break ;
					
				case MAP_BUTTON :
					EnableWindow( shipButton, false ) ;
					EnableWindow( mapButton, false ) ;
					DisplaySetMap() ;
					break ;
				
				case SET_SHIP_NUMBER_OK :
					CheckShipNumberSetting() ;
					break ;
					
				case SET_SHIP_OK :
					CheckShipSetting() ;
					break ;
				
				case SET_MAP_OK :
					CheckMapSetting( 1, 26 ) ;
					break ;
				
			} // switch
			
			break ;
			
		} // WM_COMMAND
		
		case WM_CLOSE :
			EnableWindow( hMainWindow, true ) ;
			DestroyWindow( hwnd ) ;
			break ;
			
		case WM_DESTROY :
			EnableWindow( hMainWindow, true ) ;
			break ;
				
		default :
			return DefWindowProcW( hwnd, msg, wP, lp ) ;	
	
	} // switch
	
} // SettingDialogProcedure()

void DisplaySetShip() {
	
	CreateWindow( "Static", " ship number : ", WS_VISIBLE | WS_CHILD, 200, 40, 100, 50, hSettingDlg, (HMENU)10, NULL, NULL ) ;
	shipNumTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 300, 40, 50, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	shipNumCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 360, 40, 10, 20, hSettingDlg, NULL, NULL, NULL ) ;
	messageWnd = CreateWindow( "Static", "<- Set First", WS_VISIBLE | WS_CHILD, 380, 40, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	
	setShipNumberButton = CreateWindow( "Button", "OK", WS_VISIBLE | WS_CHILD, 460, 30, 50, 20, hSettingDlg, (HMENU)SET_SHIP_NUMBER_OK, NULL, NULL ) ;
	
	CreateWindow( "Static", "        name : ", WS_VISIBLE | WS_CHILD, 200, 70, 100, 50, hSettingDlg, (HMENU)10, NULL, NULL ) ;
	nameTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 300, 70, 200, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	nameCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 510, 70, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	EnableWindow( nameTextWnd, false ) ;
	
	CreateWindow( "Static", "   size (>0) : ", WS_VISIBLE | WS_CHILD, 200, 100, 100, 50, hSettingDlg, (HMENU)10, NULL, NULL ) ;
	sizeTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 300, 100, 50, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	sizeCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 360, 100, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	EnableWindow( sizeTextWnd, false ) ;
	
	CreateWindow( "Static", "   move (>0) : ", WS_VISIBLE | WS_CHILD, 200, 130, 100, 50, hSettingDlg, (HMENU)10, NULL, NULL ) ;
	mvTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 300, 130, 50, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	mvCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 360, 130, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	EnableWindow( mvTextWnd, false ) ;
	
	CreateWindow( "Static", " break sound : ", WS_VISIBLE | WS_CHILD, 200, 160, 100, 50, hSettingDlg, (HMENU)10, NULL, NULL ) ;
	bsTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 300, 160, 200, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	bsCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 510, 160, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	EnableWindow( bsTextWnd, false ) ;
	
	setShipButton = CreateWindow( "Button", "OK", WS_VISIBLE | WS_CHILD, 410, 190, 100, 50, hSettingDlg, (HMENU)SET_SHIP_OK, NULL, NULL ) ;
	EnableWindow( setShipButton, false ) ;
	
} // DisplaySetShip()

void CheckShipNumberSetting() {
	
	char shipNumberStr[20] ;
	
	GetWindowText( shipNumTextWnd, shipNumberStr, sizeof( shipNumberStr ) ) ;
	
	if ( ! GA::IsInteger( GA::CharStringToString( shipNumberStr ) ) ) {
		SetWindowText( shipNumCondition, "X" ) ;
	} // if
	else {
		totalShip = GA::StringToInt( GA::CharStringToString( shipNumberStr ) ) ;
		if ( totalShip < 0 ) {
			SetWindowText( shipNumCondition, "X" ) ;
		} // if
		else {
			SetWindowText( shipNumCondition, "O" ) ;
			EnableWindow( shipNumTextWnd, false ) ;
			DestroyWindow( setShipNumberButton ) ;
			EnableWindow( nameTextWnd, true ) ;
			EnableWindow( sizeTextWnd, true ) ;
			EnableWindow( mvTextWnd, true ) ;
			EnableWindow( bsTextWnd, true ) ;
			EnableWindow( setShipButton, true ) ;
			existShip = 0 ;
			SetWindowText( messageWnd, "Ship 1" ) ;
		} // else
	} // else
	
} // CheckShipNumberSetting()

void CheckShipSetting() {
	
	bool canPush = true ;
	char nameStr[100], sizeStr[20], mvStr[20], bsStr[100] ;
	int sizeNum = 0, mvNum = 0 ;
	
	GetWindowText( nameTextWnd, nameStr, sizeof( nameStr ) ) ;
	GetWindowText( sizeTextWnd, sizeStr, sizeof( sizeStr ) ) ;
	GetWindowText( mvTextWnd, mvStr, sizeof( mvStr ) ) ;
	GetWindowText( bsTextWnd, bsStr, sizeof( bsStr ) ) ;
	
	SetWindowText( nameCondition, "O" ) ;
	
	if ( ! GA::IsInteger( GA::CharStringToString( sizeStr ) ) ) {
		SetWindowText( sizeCondition, "X" ) ;
		canPush = false ;
	} // if
	else {
		sizeNum = GA::StringToInt( GA::CharStringToString( sizeStr ) ) ;
		if ( sizeNum < 1 ) {
			SetWindowText( sizeCondition, "X" ) ;
			canPush = false ;
		} // if
		else {
			SetWindowText( sizeCondition, "O" ) ;
		} // else
	} // else
	
	if ( ! GA::IsInteger( GA::CharStringToString( mvStr ) ) ) {
		SetWindowText( mvCondition, "X" ) ;
		canPush = false ;
	} // if
	else {
		mvNum = GA::StringToInt( GA::CharStringToString( mvStr ) ) ;
		if ( mvNum < 1 ) {
			SetWindowText( mvCondition, "X" ) ;
			canPush = false ;
		} // if
		else {
			SetWindowText( mvCondition, "O" ) ;
		} // else
	} // else
	
	SetWindowText( bsCondition, "O" ) ;
	
	if ( canPush ) {
		
		game->PushInTemp( existShip, GA::CharStringToString( nameStr ), sizeNum, mvNum, GA::CharStringToString( bsStr ) ) ;		
		existShip++ ;
		
		SetWindowText( nameTextWnd, "" ) ;
		SetWindowText( sizeTextWnd, "" ) ;
		SetWindowText( mvTextWnd, "" ) ;
		SetWindowText( bsTextWnd, "" ) ;
		
		if ( existShip == totalShip ) {
			EnableWindow( hSettingDlg, false ) ;
			game->SetShip() ;		
			MessageBeep( MB_ICONINFORMATION ) ;	
			MessageBox( hMainWindow, "Complete !", "Condition", MB_OK ) ;
			SetWindowText( hTitleWindow, game->Introduction().c_str() ) ;	
			SetWindowText( hWarningWindow, warningStr.c_str() ) ;
			DestroyWindow( hSettingDlg ) ;
			return ;
		} // if
		
		SetWindowText( messageWnd, ( "Ship " + GA::IntToString( existShip + 1 ) ).c_str() ) ;
		
	} // if
	
} // CheckShipSetting()

void DisplaySetMap() {
	
	CreateWindow( "Static", "length (X) of the map : \n(range : 1 ~ 26)", WS_VISIBLE | WS_CHILD, 200, 50, 200, 50, hSettingDlg, NULL, NULL, NULL ) ;
	xTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 350, 50, 50, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	xCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 410, 50, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
					
	CreateWindow( "Static", " width (Y) of the map : \n(range : 1 ~ 26)", WS_VISIBLE | WS_CHILD, 200, 100, 200, 50, hSettingDlg, NULL, NULL, NULL ) ;
	yTextWnd = CreateWindow( "Edit", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 350, 100, 50, 20, hSettingDlg, NULL, NULL, NULL ) ; 
	yCondition = CreateWindow( "Static", "?", WS_VISIBLE | WS_CHILD, 410, 100, 100, 20, hSettingDlg, NULL, NULL, NULL ) ;
	
	CreateWindow( "Button", "OK", WS_VISIBLE | WS_CHILD, 410, 150, 100, 50, hSettingDlg, (HMENU)SET_MAP_OK, NULL, NULL ) ;
	
} // DisplaySetShip()

void CheckMapSetting( int min, int max ) { // [min,max]
	
	bool canSet = true ;
	char xInput[20], yInput[20] ;
	int xNum = 0, yNum = 0 ;
	
	GetWindowText( xTextWnd, xInput, sizeof( xInput ) ) ;
	GetWindowText( yTextWnd, yInput, sizeof( yInput ) ) ;
	
	if ( ! GA::IsInteger( GA::CharStringToString( xInput ) ) ) {
		SetWindowText( xCondition, "X" ) ;
		canSet = false ;
	} // if
	else {
		xNum = GA::StringToInt( GA::CharStringToString( xInput ) ) ;
		if ( xNum < min || xNum > max ) {
			SetWindowText( xCondition, "X" ) ;
			canSet = false ;
		} // if
		else {
			SetWindowText( xCondition, "O" ) ;
		} // else
	} // else
	
	if ( ! GA::IsInteger( GA::CharStringToString( yInput ) ) ) {
		SetWindowText( yCondition, "X" ) ;
		canSet = false ;
	} // if
	else {
		yNum = GA::StringToInt( GA::CharStringToString( yInput ) ) ;
		if ( yNum < min || yNum > max ) {
			SetWindowText( yCondition, "X" ) ;
			canSet = false ;
		} // if
		else {
			SetWindowText( yCondition, "O" ) ;
		} // else
	} // else
	
	if ( canSet ) {
		EnableWindow( hSettingDlg, false ) ;
		game->SetMap( xNum, yNum ) ;		
		MessageBeep( MB_ICONINFORMATION ) ;	
		MessageBox( hMainWindow, "Complete !", "Condition", MB_OK ) ;
		SetWindowText( hTitleWindow, game->Introduction().c_str() ) ;	
		SetWindowText( hWarningWindow, warningStr.c_str() ) ;
		DestroyWindow( hSettingDlg ) ;
	} // if
	
} // CheckMapSetting()

















