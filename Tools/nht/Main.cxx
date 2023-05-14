#include <vector>
#include <mutex>
#include <thread>
#include <filesystem>

#include <clang-c/Index.h>

#include <Runtime/Runtime.h>

using namespace std;
using namespace std::filesystem;

enum NeFieldType
{
	FT_UNKNOWN,
	FT_UINT8,
	FT_UINT16,
	FT_UINT32,
	FT_UINT64,
	FT_INT8,
	FT_INT16,
	FT_INT32,
	FT_INT64,
	FT_FLOAT,
	FT_DOUBLE,
	FT_VECTOR3,
	FT_VECTOR4,
	FT_MATRIX,
	FT_STRING,
	FT_BOOL,
	FT_ARRAY
};

struct NeFieldInfo
{
	enum NeFieldType type;
	char name[256];
};

struct NeComponentInfo
{
	char name[256];
	vector<NeFieldInfo> fieldInfo;
};

struct NeEnumValue
{
	char name[256];
	int64_t value;
};

struct NeEnumInfo
{
	char name[256];
	vector<NeEnumValue> values;
};

struct NeStringHash
{
	char name[256];
	uint64_t hash;
};

struct SourceLocations
{
	CXTranslationUnit tu;
	vector<CXSourceLocation> component;
	vector<CXSourceLocation> hash;
	vector<tuple<string, CXSourceLocation>> macros;
};

CXIndex _index;
vector<const char *> _cArguments, _cxxArguments;
vector<NeComponentInfo> _componentInfo;
vector<NeEnumInfo> _enumInfo;
vector<string> _processedFiles;
vector<NeStringHash> _stringHashes;
mutex _infoMutex, _fileMutex;

static inline void ProcessStruct(CXCursor cur, vector<CXSourceLocation> *componentLocations);
static inline void ProcessEnum(CXCursor cur);
static inline NeFieldType StringToType(const char *type);
static inline const char *TypeToString(NeFieldType type);
static inline bool ValidatePath(const char *path);
static inline void AddStringHash(const char *str);
static inline string MacroValue(const string &macro, const vector<tuple<string, CXSourceLocation>> &macros, CXTranslationUnit tu);

CXChildVisitResult
Visitor(CXCursor cur, CXCursor parent, CXClientData data)
{
	SourceLocations *loc = (SourceLocations *)data;

	if (clang_isCursorDefinition(cur)) {
		if (cur.kind == CXCursor_ClassDecl || cur.kind == CXCursor_StructDecl)
			ProcessStruct(cur, &loc->component);
		else if (cur.kind == CXCursor_EnumDecl)
			ProcessEnum(cur);
	} else if (cur.kind == CXCursor_VarDecl) {
	} else if (cur.kind == CXCursor_MacroExpansion) {
		CXString nameStr = clang_getCursorSpelling(cur);
		const char *name = clang_getCString(nameStr);

		size_t len = strlen(name);
		if (!strncmp("NE_COMPONENT_BASE", name, len)) {
			loc->component.push_back(clang_getCursorLocation(cur));
		} else if (!strncmp("RT_HASH", name, len)) {
			loc->hash.push_back(clang_getCursorLocation(cur));
		} else {
			/*CXToken *tokens;
			unsigned int tokenCount;
			clang_tokenize(loc->tu, clang_getCursorExtent(cur), &tokens, &tokenCount);

			for (uint32_t i = 0; i < tokenCount; ++i) {
				CXString s = clang_getTokenSpelling(loc->tu, tokens[2]);
				printf("mt = %s\n", clang_getCString(s));
				clang_disposeString(s);
			}*/

			tuple<string, CXSourceLocation> t{};

			CXString s = clang_getCursorSpelling(cur);

			get<0>(t) = clang_getCString(s);
			get<1>(t) = clang_getCursorLocation(cur);

			clang_disposeString(s);

			loc->macros.push_back(t);
		}

		clang_disposeString(nameStr);
	}
	

	return CXChildVisit_Continue;
}

CXChildVisitResult
StructVisitor(CXCursor cur, CXCursor parent, CXClientData data)
{
	NeComponentInfo *ci = (NeComponentInfo *)data;

	if (cur.kind == CXCursor_FieldDecl) {
		CXType t = clang_getCursorType(cur);

		CXString typeStr = clang_getTypeSpelling(t);
		CXString nameStr = clang_getCursorSpelling(cur);

		const char *name = clang_getCString(nameStr);
		const char *type = clang_getCString(typeStr);

		if (name[0] != '_') {
			NeFieldInfo fi{};

			snprintf(fi.name, sizeof(fi.name), "%s", name);
			fi.type = StringToType(type);

			ci->fieldInfo.push_back(fi);
		}

		clang_disposeString(typeStr);
		clang_disposeString(nameStr);
	}

	return CXChildVisit_Continue;
}

CXChildVisitResult
EnumVisitor(CXCursor cur, CXCursor parent, CXClientData data)
{
	NeEnumInfo *ei = (NeEnumInfo *)data;
	
	CXType t = clang_getCursorType(cur);

	if (t.kind == CXType_Int || t.kind == CXType_UInt) {
		CXString nameStr = clang_getCursorSpelling(cur);
		const char *name = clang_getCString(nameStr);

		NeEnumValue ev{};
		snprintf(ev.name, sizeof(ev.name), "%s", name);
		ev.value = clang_getEnumConstantDeclValue(cur);

		ei->values.push_back(ev);

		clang_disposeString(nameStr);
	}

	return CXChildVisit_Continue;
}

static void
Parse(const char *file)
{
	SourceLocations loc;

	CXTranslationUnit tu = strstr(file, ".cxx") ?
	clang_createTranslationUnitFromSourceFile(_index, file, (int)_cxxArguments.size(), _cxxArguments.data(), 0, NULL)
	:
	clang_createTranslationUnitFromSourceFile(_index, file, (int)_cArguments.size(), _cArguments.data(), 0, NULL);

	//CXTranslationUnit tu = clang_parseTranslationUnit(_index, file, _clangArguments.data(), (int)_clangArguments.size(), NULL, 0, CXTranslationUnit_DetailedPreprocessingRecord);
	clang_visitChildren(clang_getTranslationUnitCursor(tu), Visitor, &loc);

	for (const CXSourceLocation &sl : loc.hash) {
		CXCursor cur = clang_getCursor(tu, sl);

		CXToken *tokens;
		unsigned int tokenCount;
		clang_tokenize(tu, clang_getCursorExtent(cur), &tokens, &tokenCount);

		if (clang_getTokenKind(tokens[2]) == CXToken_Literal) {
			CXString s = clang_getTokenSpelling(tu, tokens[2]);
			const char *str = clang_getCString(s);

			if (strlen(str))
				AddStringHash(str);

			clang_disposeString(s);
		} else if (clang_getTokenKind(tokens[2]) == CXToken_Identifier) {
			CXCursor c = clang_getCursor(tu, clang_getTokenLocation(tu, tokens[2]));

			if (clang_getCursorKind(c) == CXCursor_MacroExpansion) {
				CXString s = clang_getTokenSpelling(tu, tokens[2]);
				const char *str = clang_getCString(s);

				MacroValue(str, loc.macros, tu);

				clang_disposeString(s);
			}
		}

		clang_disposeTokens(tu, tokens, tokenCount);
	}

	clang_disposeTranslationUnit(tu);
}

static void
ProcessDirectory(const char *path, vector<string> &files)
{
	for (const directory_entry &dent : recursive_directory_iterator(path)) {
		if (dent.is_directory()) {
			ProcessDirectory(dent.path().string().c_str(), files);
		} else if (dent.is_regular_file()) {
			string p = dent.path().string();

			bool exists = false;
			for (const string &f : files) {
				if (f.compare(p))
					continue;

				exists = true;
				break;
			}

			if (exists)
				continue;

			const char *e = strrchr(p.c_str(), '.');
			if (!e || (strncmp(e, ".h", 2) && strncmp(e, ".c", 2) && strncmp(e, ".cxx", 2)))
				continue;

			files.push_back(p);
		}
	}
}

static void
ProcessFiles(const vector<string> &files)
{

}

int
main(int argc, char *argv[])
{
	printf("NekoEngine Header Tool\n");

	_index = clang_createIndex(0, 0);

	vector<const char *> arguments{};
	arguments.push_back("-I/Users/alex/Projects/NekoEngine/Include");
	arguments.push_back("-I/Users/alex/Projects/NekoEngine/Deps");
	arguments.push_back("-I/Users/alex/Projects/NekoEngine/Deps/Lua");
	arguments.push_back("-I/Users/alex/Projects/NekoEngine/Deps/PhysFS");

	_cArguments.push_back("-std=c18");
	copy(arguments.begin(), arguments.end(), back_inserter(_cArguments));

	_cxxArguments.push_back("-std=c++20");
	copy(arguments.begin(), arguments.end(), back_inserter(_cxxArguments));

	vector<string> files;
	ProcessDirectory("/Users/alex/Projects/NekoEngine/Engine", files);

	uint32_t threadCount = thread::hardware_concurrency();
	uint64_t bucket = files.size() / threadCount;

	thread *threads = new thread[threadCount];
	for (uint32_t i = 0; i < threadCount; ++i) {
		threads[i] = thread([i, bucket, files] {
			uint64_t start = i * bucket;
			for (int j = 0; j < bucket; ++j)
				Parse(files[start + j].c_str());
		});
	}

	for (uint32_t i = 0; i < threadCount; ++i)
		threads[i].join();

	for (uint64_t i = bucket * threadCount; i < files.size(); ++i)
		Parse(files[i].c_str());

	delete[] threads;

	printf("Found %zu components\n", _componentInfo.size());
	for (const NeComponentInfo &ci : _componentInfo) {
		printf("Component: %s\n", ci.name);

		for (const NeFieldInfo &fi : ci.fieldInfo)
			printf("\t%s %s\n", TypeToString(fi.type), fi.name);
	}

	printf("Found %zu enums\n", _enumInfo.size());
	/*for (const NeEnumInfo &ei : _enumInfo) {
		printf("Enum: %s\n", ei.name);

		for (const NeEnumValue &ev : ei.values)
			printf("\t%s = %lld\n", ev.name, ev.value);
	}*/

	printf("Found %zu string hashes\n", _stringHashes.size());
	for (const NeStringHash &sh : _stringHashes) {
		printf("%s = %llu\n", sh.name, sh.hash);
	}

	clang_disposeIndex(_index);

	return 0;
}

static inline void
ProcessStruct(CXCursor cur, vector<CXSourceLocation> *componentLocations)
{
	CXSourceRange range = clang_getCursorExtent(cur);

	CXFile f;
	clang_getFileLocation(clang_getRangeStart(range), &f, NULL, NULL, NULL);
	
	CXString fName = clang_getFileName(f);
	if (!ValidatePath(clang_getCString(fName))) {
		clang_disposeString(fName);
		return;
	}
	clang_disposeString(fName);

	NeComponentInfo ci{};

	CXString name = clang_getCursorSpelling(cur);
	//uint64_t hash = Rt_HashString(clang_getCString(name));

	snprintf(ci.name, sizeof(ci.name), "%s", clang_getCString(name));
	clang_disposeString(name);

	for (const NeComponentInfo &eci : _componentInfo)
		if (!strncmp(eci.name, ci.name, 256))
			return;

	unsigned s, e;
	clang_getExpansionLocation(clang_getRangeStart(range), &f, &s, NULL, NULL);
	clang_getExpansionLocation(clang_getRangeEnd(range), NULL, &e, NULL, NULL);

	size_t i = 0;
	bool component = false;
	for (i = 0; i < componentLocations->size(); ++i) {
		CXFile cf;
		unsigned l;

		clang_getExpansionLocation((*componentLocations)[i], &cf, &l, NULL, NULL);

		if (!clang_File_isEqual(f, cf))
			continue;

		if (s < l && l < e) {
			component = true;
			break;
		}
	}

	if (!component)
		return;

	componentLocations->erase(componentLocations->begin() + i);

	_infoMutex.lock();
	_componentInfo.push_back(ci);
	clang_visitChildren(cur, StructVisitor, &_componentInfo[_componentInfo.size() - 1]);
	_infoMutex.unlock();
}

static inline void
ProcessEnum(CXCursor cur)
{
	CXSourceRange range = clang_getCursorExtent(cur);

	CXFile f;
	clang_getFileLocation(clang_getRangeStart(range), &f, NULL, NULL, NULL);
	
	CXString fName = clang_getFileName(f);
	if (!ValidatePath(clang_getCString(fName))) {
		clang_disposeString(fName);
		return;
	}
	clang_disposeString(fName);

	NeEnumInfo ei{};

	CXString nameStr = clang_getCursorSpelling(cur);
	const char *name = clang_getCString(nameStr);

	if (name[0] == '_' || !strlen(name) || !strncmp("tag", name, 3))
		goto exit;

	for (const NeEnumInfo &eei : _enumInfo)
		if (!strncmp(eei.name, name, 256))
			goto exit;

	snprintf(ei.name, sizeof(ei.name), "%s", name);

	_infoMutex.lock();
	_enumInfo.push_back(ei);
	clang_visitChildren(cur, EnumVisitor, &_enumInfo[_enumInfo.size() - 1]);
	_infoMutex.unlock();

exit:
	clang_disposeString(nameStr);
}

static inline NeFieldType
StringToType(const char *type)
{
	size_t len = strlen(type);
	if (!strncmp(type, "uint8_t", len))
		return FT_UINT8;
	else if (!strncmp(type, "uint16_t", len))
		return FT_UINT16;
	else if (!strncmp(type, "uint32_t", len))
		return FT_UINT32;
	else if (!strncmp(type, "uint64_t", len))
		return FT_UINT64;
	else if (!strncmp(type, "int8_t", len))
		return FT_INT8;
	else if (!strncmp(type, "int16_t", len))
		return FT_INT16;
	else if (!strncmp(type, "int32_t", len))
		return FT_INT32;
	else if (!strncmp(type, "int64_t", len))
		return FT_INT64;
	else if (!strncmp(type, "float", len))
		return FT_FLOAT;
	else if (!strncmp(type, "double", len))
		return FT_DOUBLE;
	else if (!strncmp(type, "struct NeVec3", len))
		return FT_VECTOR3;
	else if (!strncmp(type, "struct NeVec4", len))
		return FT_VECTOR4;
	else if (!strncmp(type, "struct NeMatrix", len))
		return FT_MATRIX;
	else if (!strncmp(type, "const char *", len))
		return FT_STRING;
	else if (!strncmp(type, "bool", len))
		return FT_BOOL;
	else if (!strncmp(type, "struct NeArray", len))
		return FT_ARRAY;
	else
		return FT_UNKNOWN;
}

static inline const char *
TypeToString(NeFieldType type)
{
	switch (type) {
	case FT_UINT8: return "ui8";
	case FT_UINT16: return "ui16";
	case FT_UINT32: return "ui32";
	case FT_UINT64: return "ui64";
	case FT_INT8: return "i8";
	case FT_INT16: return "i16";
	case FT_INT32: return "i32";
	case FT_INT64: return "i64";
	case FT_FLOAT: return "flt";
	case FT_DOUBLE: return "dbl";
	case FT_VECTOR3: return "vec3";
	case FT_VECTOR4: return "vec4";
	case FT_MATRIX: return "matrix";
	case FT_STRING: return "str";
	case FT_BOOL: return "bool";
	case FT_ARRAY: return "array";
	default: return "unknown";
	}
}

static inline bool
ValidatePath(const char *path)
{
	return strstr(path, "NekoEngine");
}

static inline void
AddStringHash(const char *str)
{
	NeStringHash sh{};

	if (str[0] == '"')
		++str;

	snprintf(sh.name, sizeof(sh.name), "%s", str);

	uint64_t last = strlen(sh.name) - 1;
	if (sh.name[last] == '"')
		sh.name[last] = 0x0;

	sh.hash = Rt_HashString(sh.name);
	_stringHashes.push_back(sh);
}

static inline string
MacroValue(const string &macro, const vector<tuple<string, CXSourceLocation>> &macros, CXTranslationUnit tu)
{
	for (const tuple<string, CXSourceLocation> &t : macros) {
		if (macro.compare(get<0>(t)))
			continue;

		CXCursor cur = clang_getCursor(tu, get<1>(t));

		CXToken *tokens;
		unsigned int tokenCount;
		clang_tokenize(tu, clang_getCursorExtent(cur), &tokens, &tokenCount);

		for (uint32_t i = 0; i < tokenCount; ++i) {
			CXString s = clang_getTokenSpelling(tu, tokens[i]);
			printf("mt = %s\n", clang_getCString(s));
			clang_disposeString(s);
		}
	}
	return string();
}

/* NekoEngine Header Tool
 *
 * Main.cxx
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
