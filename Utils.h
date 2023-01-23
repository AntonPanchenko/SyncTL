#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

enum
{
	UTILS_ERROR_BASE = 1,
	CHESS_ERROR_BASE = 1000,
	GUI_ERROR_BASE = 2000,
	XML_ERROR_BASE = 3000,
	CHESS_TREE_ERROR_BASE = 4000,
	CHESS_PLAYER_ERROR_BASE = 5000,
	SYNCHRONIZATION_ERROR_BASE = 6000,
	THREADING_ERROR_BASE = 7000,
	TIMER_ERROR_BASE = 8000,
	UNIFORM_ALLOCATOR_ERROR_BASE = 9000,
	BASE_FOR_ANOTHER_ERROR_CLASSES = 10000
};

enum
{
	ERR_OK = 0,
	UNDEFINED_ERROR = -1
};

enum
{
	GUI_ERR_OK = 0,
	GUI_ERR_CANNOT_LOAD_IMAGE = GUI_ERROR_BASE + 1,
	GUI_ERR_NO_SKINS,
	GUI_ERR_INVALID_PARAMETERS,
	GUI_ERR_CANNOT_CREATE_MENU,
	GUI_ERR_CANCELLED_BY_USER,
	GUI_ERR_TREE_DEPTH_SPECIFIED_IS_TOO_LOW,
	GUI_ERR_TREE_DEPTH_SPECIFIED_IS_TOO_HIGH,
	GUI_ERR_TREE_DEPTH_INVALID,
	GUI_ERR_INVALID_URL
};

/*enum
{
	UI_ERR_OK = 0,
	UI_ERR_CANNOT_LOAD_IMAGE = GUI_ERROR_BASE,
	UI_ERR_CANNOT_CREATE_MENU
};*/

enum
{
	UTILS_ERROR_UNSUPPORTED_INT_SIZE = UTILS_ERROR_BASE + 1,
	UTILS_ERROR_UNDEFINED_ERROR = -1,
	UTILS_ERROR_STD_EXCEPTION
};

enum
{
	ERROR_MSG_BUFFER_LENGTH = 0x7F
};

void debug_break();
#ifdef _DEBUG
#define ASSERT(condition)	\
if (condition != true)	\
	{						\
	debug_break();		\
	}
#else
#define ASSERT(condition)
#endif //DEBUG

#ifndef NULL
#define NULL 0//((void*)0)
#endif//

class Exception
{
public:
	Exception(unsigned int error_code,
		const wchar_t* message, 
		const char* filename,
		unsigned int line,
		unsigned int system_error_code = 0) :
		m_error_code(error_code), 
		m_message(message), 
		m_filename(filename),
		m_line(line),
		m_system_error_code(system_error_code)
	{}
	inline unsigned int GetErrorCode() const
	{
		return m_error_code; 
	}
	inline const wchar_t* GetErrorMessage() const
	{
		return m_message;
	}
	inline const char* GetFilename() const	//macros _LINE_ creates ansi string
	{
		return m_filename;
	}
	inline unsigned int GetLine() const
	{
		return m_line;
	}
	inline unsigned int GetSystemErrorCode() const
	{
		return m_system_error_code;
	}
protected:
	unsigned int m_error_code;
	const wchar_t* m_message;
	const char* m_filename;
	unsigned int m_line;
	unsigned int m_system_error_code;
};

//this is to use in Exception constructor
#define EXC_HERE	__FILE__, __LINE__

extern const wchar_t* ERR_MSG_CANNOT_LOAD_IMAGE;

#if defined(NOGUI)
//to log. later.
#define ReportMessage
#define ReportError
#else
#define ReportMessage ShowMessageDialog
#define ReportError ShowErrorDialog
#endif

const wchar_t* GetErrorMessage(unsigned int error_code);
//both ShowMessageDialog functons are modal
unsigned int /*return code*/ _ShowMessageDialog(unsigned int error_code, const wchar_t* details = 0);
unsigned int /*return code*/ _ShowMessageDialog(const wchar_t* message, const wchar_t* details = 0);
unsigned int /*return code*/ _ShowErrorDialog(const wchar_t* message, const Exception& exc);

void InternalOutputDebugMsg(const wchar_t* msg);

#ifdef _DEBUG
#define OutputDebugMsg(msg)	InternalOutputDebugMsg(msg)
#else
#define OutputDebugMsg(msg)
#endif //DEBUG_

//built-in "int" size depends on architecture.
#define BIT_SIZEOF_INT	sizeof(int) << 3

template <typename CharType>
unsigned int TStrLen(CharType* str)
{
	unsigned int ret_val(0);
	while (*str != NULL)
	{
		++ret_val;
	}
	return ret_val;
}

//unsigned int /*copied size*/ ByteCopy(const char* src, char* dst, unsigned int size);

#endif //UTILS_H_INCLUDED