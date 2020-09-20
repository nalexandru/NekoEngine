#include <vector>

#include <clang-c/Index.h>

using namespace std;

CXIndex _index;
vector<const char *> _clangArguments;

static inline void _processStruct(CXCursor cur);
static inline void _processEnum(CXCursor cur);

CXChildVisitResult
_visitor(CXCursor cur, CXCursor parent, CXClientData data)
{
	if (clang_isCursorDefinition(cur)) {
		if (cur.kind == CXCursor_ClassDecl || cur.kind == CXCursor_StructDecl)
			_processStruct(cur);
		else if (cur.kind == CXCursor_EnumDecl)
			_processEnum(cur);
	} else {
		// variables (config maybe ?)
	}

	if (cur.kind == CXCursor_LastPreprocessing)
		return CXChildVisit_Break;

	return CXChildVisit_Continue;
}

void
parse(const char *file)
{
	CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(_index, file, (int)_clangArguments.size(), _clangArguments.data(), 0, NULL);
	CXCursor cur = clang_getTranslationUnitCursor(tu);

	clang_visitChildren(cur, _visitor, NULL);
}

int
main(int argc, char *argv[])
{
	_index = clang_createIndex(0, 0);
}

static inline void
_processStruct(CXCursor cur)
{
	//
}

static inline void
_processEnum(CXCursor cur)
{

}
