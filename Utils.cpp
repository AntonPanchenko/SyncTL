#include "Utils.h"
#ifdef BUILDING_CHESS_MAIN_MODULE
#include "../Engine/Chess.h"
#include "../Chess/ChessBoardUI.h"
#endif //BUILDING_CHESS_MAIN_MODULE
#include <Windows.h>
#ifdef QT_VERSION
#include <QMessageBox>
#include <QString>
#endif //QT_VERSION

//#define _ITERATOR_DEBUG_LEVEL	0

void debug_break()
{
	DebugBreak();
}

//error messages strings
//of course I could load this strings from resources, but this is error handling,
//better this strings to be here
#ifdef BUILDING_CHESS_MAIN_MODULE
const wchar_t* err_msg_cannot_load_image = L"Cannot Load Image";
const wchar_t* err_msg_cell_occupied = L"Cannot go there, cell is already occupied";
const wchar_t* err_msg_cell_has_no_piece = L"Cell has no chess piece";
const wchar_t* err_msg_piece_cannot_go_like_that = L"Chess piece cannot go like that";
const wchar_t* err_msg_cannot_co_out_of_board = L"Chess piece cannot go out of board";
const wchar_t* err_msg_cannot_go_already_there = L"Cannot go, already standing there";
const wchar_t* err_msg_invalid_chess_address = L"Invalid Chess Address";
const wchar_t* err_msg_piece_not_on_board = L"Chess piece is not on board";
const wchar_t* err_msg_no_piece_specified = L"No Piece Specified";
const wchar_t* err_gui_msg_cannot_load_image = L"Cannot load image";
const wchar_t* err_gui_wrong_turn = L"Wrong turn. now another color must move";
#else
#endif //BUILDING_CHESS_MAIN_MODULE
const wchar_t* err_msg_error_with_no_error_msg = L"Don't have error message for this error";

const wchar_t* GetErrorMessage(unsigned int error_code)
{
	switch (error_code)
	{
	#ifdef BUILDING_CHESS_MAIN_MODULE
	case CH_ERR_CELL_OCCUPIED: return err_msg_cell_occupied;
	case CH_ERR_CELL_HAS_NO_PIECE: return err_msg_cell_has_no_piece;
	case CH_ERR_PIECE_CANNOT_GO_LIKE_THAT: return err_msg_piece_cannot_go_like_that;
	case CH_ERR_CANNOT_GO_OUT_OF_BOARD: return err_msg_cannot_co_out_of_board;
	case CH_ERR_CANNOT_GO_ALREADY_THERE: return err_msg_cannot_go_already_there;
	case CH_ERR_INVALID_CHESS_ADDRESS: return err_msg_invalid_chess_address;
	case CH_ERR_PIECE_NOT_ON_BOARD: return err_msg_piece_not_on_board;
	case CH_ERR_NO_PIECE_SPECIFIED: return err_msg_no_piece_specified;
	case CH_ERR_WRONG_TURN: return err_gui_wrong_turn;
	case GUI_ERR_CANNOT_LOAD_IMAGE: return err_gui_msg_cannot_load_image;
	#else
	#endif //BUILDING_CHESS_MAIN_MODULE
	default: return err_msg_error_with_no_error_msg;
	}
}

#if !defined (NOGUI)

#ifdef QT_VERSION
//no reason to show dlg on server
unsigned int /*return code*/ ShowMessageDialog(unsigned int error_code, const wchar_t* details)
{
	const wchar_t* msg = GetErrorMessage(error_code);
	ASSERT(msg != NULL);
	return ShowMessageDialog(msg, details);
}
unsigned int /*return code*/ ShowMessageDialog(const wchar_t* message, const wchar_t* details)
{
	QString title("Error Message");
	QString text("Message: ");
	text.append((QChar*)message, (int)wcslen(message));
	if (details != NULL)
	{
		unsigned int details_length = (unsigned int)wcslen(details);
		if (details_length > 0)
		{
			text.append("\nDetails: ");
			text.append(((QChar*)details), details_length);
		}
	}
	QMessageBox message_box(QMessageBox::Critical,
							title,
							text,
							QMessageBox::Close);
	message_box.exec();
	return GUI_ERR_OK;
}

unsigned int /*return code*/ ShowErrorDialog(const wchar_t* message, const Exception& exc)
{
	const wchar_t* error_message = GetErrorMessage(exc.GetErrorCode());
	const wchar_t* exc_message = exc.GetErrorMessage();
#ifdef _WIN64
	wchar_t exc_system_error_code[19 /*0x-01234567-01234567 and 0*/];
	memset(exc_system_error_code, 0, sizeof(exc_system_error_code));
	wsprintf(exc_system_error_code, L"0x%L", exc_system_error_code);
#else
	wchar_t exc_system_error_code[11 /*0x-01234567 and 0*/];
	memset(exc_system_error_code, 0, sizeof(exc_system_error_code));
	wsprintf(exc_system_error_code, L"0x%x", exc_system_error_code);
#endif //_WIN64
	//const wchar_t* err_msg_title = "Error message: ";
	const wchar_t exc_msg_title[] = L"Exception message: ";
	const wchar_t exc_msg_src_file_name_title[] = L"Source file name: ";
	const wchar_t exc_msg_sys_err_code_title[] = L"System error code: ";
	const wchar_t exc_msg_line_number_title[] = L"Line number: ";
	wchar_t exc_line_number[20];
	memset(exc_line_number, 0, sizeof(exc_line_number));
	wsprintf(exc_line_number, L"%d", exc.GetLine());
	const wchar_t newline[] = L"\n";
	unsigned int newline_length = (unsigned int)wcslen(newline);
	QString err_msg_qstr((QChar*)error_message);
	QString text;// ((QChar*)L"Error message");
	//text.append((QChar*)error_message, wcslen(error_message));
	//text.append((QChar*)newline, newline_length);
	text.append((QChar*)exc_msg_title, (unsigned int)wcslen(exc_msg_title));
	text.append((QChar*)newline, newline_length);
	text.append((QChar*)exc_message, (unsigned int)wcslen(exc_message));
	text.append((QChar*)newline, newline_length);
	text.append((QChar*)newline, newline_length);
	text.append((QChar*)exc_msg_src_file_name_title, (unsigned int)wcslen(exc_msg_src_file_name_title));
	text.append(exc.GetFilename());// , strlen(exc.GetFilename()));
	text.append((QChar*)newline, newline_length);
	text.append((QChar*)exc_msg_line_number_title, (unsigned int)wcslen(exc_msg_line_number_title));
	text.append((QChar*)exc_line_number, (unsigned int)wcslen(exc_line_number));
	text.append((QChar*)newline, newline_length);
	text.append((QChar*)exc_msg_sys_err_code_title, (unsigned int)wcslen(exc_msg_sys_err_code_title));
	text.append((QChar*)exc_system_error_code, (unsigned int)wcslen(exc_system_error_code));
	text.append((QChar*)newline, newline_length);
	if(message != NULL)
	{
		text.append((QChar*)newline, newline_length);
		text.append((QChar*)message, (int)wcslen(message));
	}
	//
	QMessageBox message_box(QMessageBox::Critical,
		err_msg_qstr,
		text,
		QMessageBox::Close);
	message_box.exec();
	return GUI_ERR_OK;
}
#else if defined (NOGUI)
//bad idea, in fact, better rename it somehow
unsigned int /*return code*/ ShowMessageDialog(unsigned int error_code, const wchar_t* details);
unsigned int /*return code*/ ShowMessageDialog(const wchar_t* message, const wchar_t* details);
unsigned int /*return code*/ ShowErrorDialog(const wchar_t* message, const Exception& exc)
{}
#endif //QT_VERSION

#endif //not defined NOGUI

void InternalOutputDebugMsg(const wchar_t* msg)
{
	OutputDebugString(msg);
}

/*unsigned int copied size ByteCopy(const char* src, char* dst, unsigned int size)
{
	unsigned int bytes_left = size;
	bool to_upper_address = (dst > src);
	//note that int size depends on architecture;
	if (sizeof(int) == 8)
	{
	}
	else if (sizeof(int) == 4)
	{
	}
	else {
		throw Exception(UTILS_ERROR_UNSUPPORTED_INT_SIZE,
			L"sizeof(int) != 8 and sizeof(int) != 4, ByteCopy cannot work here",
			EXC_HERE);
	}
	if (to_upper_address)
	{
		while (bytes_left != 0)
		{
			switch (bytes_left)
			{
			case 3:
			case 2:
			case 1:
			default:
				(int*)(*dst) = (int*)(*src);
			}
		}
	}
	else {
		while (bytes_left != 0)
		{
			switch (bytes_left)
			{
			case 3:
			case 2:
			case 1:
			default:
			}
		}
	}
}*/