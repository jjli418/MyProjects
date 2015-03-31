// extract_string_literals.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <assert.h>

BOOL ScanALiteral(_TCHAR* buf, int* startPos, int* endPos, int* cursor, int limit);
VOID StrToUpper(const _TCHAR* src, _TCHAR* dest, int count);
VOID WriteToLexFile (HANDLE lexHandle, const _TCHAR* literalBuf, const _TCHAR* uppercaseBuf, int countBytes);
VOID WriteToKeysFile(HANDLE keysHandle, const _TCHAR* uppercaseBuf, int countBytes);
VOID WriteToNewSyntaxFile (HANDLE hNewSyntaxFile, const _TCHAR* readBuf, _TCHAR* uppercaseBuf, int newSyntaxCursor, int startPos, int countBytes );

int _tmain(int argc, _TCHAR* argv[])
{
	char syntaxFile[] = "c:\\mytemp\\syntax.txt";
	char outputLexFile[] = "c:\\mytemp\\lex.txt";
	char outputKeyFile[] = "c:\\mytemp\\key.txt";
	char outputNewSyntaxFile[] = "c:\\mytemp\\newSyntax.txt";

	_TCHAR readBuf[1024*100];
	_TCHAR literalBuf[128];
	_TCHAR uppercaseBuf[128];
	DWORD  toRead = 1024*100;
	DWORD  bytesRead;

	HANDLE hSyntaxFile = CreateFile(TEXT(syntaxFile), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
	HANDLE hLexFile = CreateFile((LPCSTR)outputLexFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
	HANDLE hKeyFile = CreateFile((LPCSTR)outputKeyFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
	HANDLE hNewSyntaxFile = CreateFile(TEXT(outputNewSyntaxFile), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);

	BOOL res = ReadFile(hSyntaxFile, readBuf, toRead, &bytesRead, NULL);

	assert(res && bytesRead > 0);

	int startPos = 0; // start position of the literal
	int endPos = 0; // end pos os the literal
	int cursor = 0;
	int newSyntaxCursor = 0;
	int countBytes;

	do
	{

		res = ScanALiteral (readBuf, &startPos, &endPos, &cursor, bytesRead); 

		if (res)
		{
			countBytes = endPos - startPos - 1;

			if(countBytes <= 1)
				continue;

			strncpy (literalBuf, readBuf + startPos + 1,countBytes);  // copy the string

			StrToUpper (literalBuf, uppercaseBuf, countBytes);

			WriteToLexFile (hLexFile, literalBuf, uppercaseBuf, countBytes);
			WriteToKeysFile (hKeyFile, uppercaseBuf, countBytes);
			WriteToNewSyntaxFile (hNewSyntaxFile, readBuf, uppercaseBuf, newSyntaxCursor, startPos,countBytes );

			newSyntaxCursor = endPos + 1;
		}

	} while(res);

	// write remaining text
	DWORD bytesWritten;
	if (newSyntaxCursor < bytesRead)
		WriteFile(hNewSyntaxFile, readBuf + newSyntaxCursor, bytesRead - newSyntaxCursor - 1, &bytesWritten, NULL);

	CloseHandle(hSyntaxFile);
	CloseHandle(hLexFile);
	CloseHandle(hKeyFile);
	CloseHandle(hNewSyntaxFile);
	

	return 0;
}

BOOL ScanALiteral(_TCHAR* buf, int* startPos, int* endPos, int* cursor, int limit)
{
	int i = *cursor;
	BOOL res = false;
	int bPos = -1;
	int ePos = -1;

	int state = 1;

	while(true)
	{
		while(buf[i] != '\'' && i < limit) i++;

		if(i >= limit)
			return res;

		bPos = i++; // get a start pos


		while(buf[i] != '\'' && buf[i] !='\n' && i < limit) i++; // scan another one

		if(buf[i] == '\n')
			continue;


		if(i >= limit)
			return res;

		ePos = i++; // get a end pos for the literal

		break;

	}

	res = true;
	*cursor = i;
	*startPos = bPos;
	*endPos = ePos;

	return res;
}

VOID StrToUpper (const _TCHAR* src, _TCHAR* dest, int count)
{
	int i = 0;

	while (i < count)
	{
		dest[i] = toupper (src[i]);
		i++;
	}

}

VOID WriteToLexFile (HANDLE lexHandle, const _TCHAR* literalBuf, const _TCHAR* uppercaseBuf, int countBytes)
{
	int toWrite = 0;
	_TCHAR tmpBuf[256];

	strncpy (tmpBuf + toWrite, uppercaseBuf, countBytes);
	toWrite += countBytes;

	strncpy (tmpBuf + toWrite, ":", 1);
	toWrite += 1;

	strncpy (tmpBuf + toWrite, "    '", 5);
	toWrite += 5;

	strncpy (tmpBuf + toWrite, literalBuf, countBytes); 
	toWrite += countBytes;

	strncpy (tmpBuf + toWrite, "';\n", 3);
	toWrite += 3;


	DWORD writtenWords;
	WriteFile(lexHandle, tmpBuf, toWrite, &writtenWords, NULL);
}

VOID WriteToKeysFile(HANDLE keysHandle, const _TCHAR* uppercaseBuf, int countBytes)
{
	//keywords2.Add("PRINT",PRINT);
	_TCHAR buf[256];
	_TCHAR* const_str = "keywords2[\"";

	int toWrite = 0;
	
	strncpy(buf + toWrite, const_str, strlen(const_str));
	toWrite += strlen(const_str);

	strncpy(buf + toWrite, uppercaseBuf, countBytes);
	toWrite += countBytes;

	strncpy(buf + toWrite, "\"] = ", 5);
	toWrite += 5;

	strncpy(buf + toWrite, uppercaseBuf, countBytes);
	toWrite += countBytes;

	strncpy(buf + toWrite, ";\n", 2);
	toWrite += 2;

	DWORD writtenWords;
	WriteFile(keysHandle, buf, toWrite, &writtenWords, NULL);
}

VOID WriteToNewSyntaxFile (HANDLE hNewSyntaxFile, const _TCHAR* readBuf, _TCHAR* uppercaseBuf, int newSyntaxCursor, int startPos, int countBytes )
{
	DWORD writtenWords;
	
	WriteFile(hNewSyntaxFile, readBuf + newSyntaxCursor, startPos - newSyntaxCursor, &writtenWords, NULL); // copy the unchanged text

	WriteFile(hNewSyntaxFile, uppercaseBuf, countBytes, &writtenWords, NULL); // write the LEX token
}