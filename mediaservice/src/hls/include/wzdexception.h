#pragma once
#include <exception>
#include <string>
namespace wzd
{
	class IException :
		public std::exception
	{
	public:
		IException() throw() {}
		virtual~IException() throw() {}

		//virtual char* what(char)	= 0;
		//virtual wchar_t* what(wchar_t) = 0;
		virtual std::string what()  = 0;
		//* 这是STD::Exception的函数,必须重载它.
		virtual const char* what() const throw()
		{
			return what();
		}
		virtual std::wstring what_u() = 0;

		//virtual char* Solution(char)	= 0;
		//virtual wchar_t* Solution(wchar_t) = 0;
		virtual std::string Solution()	= 0;
		virtual std::wstring Solution_u() = 0;

		virtual int ErrorCode() = 0;
	protected:
	private:
	};

#define DEFINE_EXCEPTION_CLASS(className)														\
	class C##className##Exception																\
		:public wzd::IException																	\
	{																							\
	public:																						\
		explicit C##className##Exception(const std::string& psz)								\
		{																						\
			m_strWhat = psz;																	\
		}																						\
		explicit C##className##Exception(const std::wstring& psz)								\
		{																						\
			m_wstrWhat = psz;																	\
		}																						\
																								\
		C##className##Exception(const std::string& psz, int iErrCode)							\
		{																						\
			m_strWhat = psz;																	\
			m_iErrcode = iErrCode;																\
		}																						\
																								\
		C##className##Exception(const std::wstring& psz, int iErrCode)							\
		{																						\
			m_wstrWhat = psz;																	\
			m_iErrcode = iErrCode;																\
		}																						\
																								\
		C##className##Exception(const std::string& psz, int iErrCode, const std::string& sol)	\
		{																						\
			m_strSol = sol;																		\
			m_strWhat = psz;																	\
			m_iErrcode = iErrCode;																\
		}																						\
		C##className##Exception(const std::wstring& psz, int iErrCode, const std::wstring& sol)	\
		{																						\
			m_wstrSol = sol;																	\
			m_wstrWhat = psz;																	\
			m_iErrcode = iErrCode;																\
		}																						\
		virtual ~C##className##Exception(void) throw() {}										\
																								\
		virtual std::string what()																\
		{																						\
			return m_strWhat;																	\
		}																						\
        virtual const char* what() const throw()												\
		{																						\
			return m_strWhat.c_str();															\
		}																						\
		virtual std::wstring what_u()															\
		{																						\
			return m_wstrWhat;																	\
		}																						\
																								\
		virtual std::string Solution()															\
		{																						\
			return m_strSol;																	\
		}																						\
		virtual std::wstring Solution_u()														\
		{																						\
			return m_wstrSol;																	\
		}																						\
																								\
		virtual int ErrorCode()																	\
		{																						\
			return m_iErrcode;																	\
		}																						\
	private:																					\
		char*		m_pszWhat;																	\
		wchar_t*	m_pwszWhat;																	\
																								\
		std::string m_strWhat;																	\
		std::wstring m_wstrWhat;																\
																								\
		int m_iErrcode;																			\
		char*		m_pszSol;																	\
		wchar_t*	m_pwszSol;																	\
																								\
		std::string m_strSol;																	\
		std::wstring m_wstrSol;																	\
	};
}
