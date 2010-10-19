#ifndef YAML_OSTREAM_H
#define YAML_OSTREAM_H

#include "llvm/ADT/StringRef.h"
#include "yasmx/Config/export.h"

namespace YAML
{
	class YASM_LIB_EXPORT ostream
	{
	public:
		ostream();
		~ostream();
		
		void reserve(unsigned size);
		void put(char ch);
		const char *str() const { return m_buffer; }
		
		unsigned row() const { return m_row; }
		unsigned col() const { return m_col; }
		unsigned pos() const { return m_pos; }
		
	private:
		char *m_buffer;
		unsigned m_pos;
		unsigned m_size;
		
		unsigned m_row, m_col;
	};
	
	ostream& operator << (ostream& out, llvm::StringRef str);
	ostream& operator << (ostream& out, char ch);
}

#endif