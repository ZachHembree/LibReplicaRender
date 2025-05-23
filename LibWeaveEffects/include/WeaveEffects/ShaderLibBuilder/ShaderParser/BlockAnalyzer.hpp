#pragma once
#include "WeaveUtils/TextUtils.hpp"
#include "WeaveUtils/TextBlock.hpp"
#include <unordered_map>

namespace Weave::Effects
{
    using std::string_view;

    enum class LexBlockTypes : uint
    {
        Unknown = 0,

        /// <summary>
        /// Unterminated non-whitespace text block
        /// </summary>
        Unterminated = 1 << 0,

        Directive = 1 << 1,
        Separator = 1 << 2,
        Preamble = 1 << 3,
        Container = 1 << 4,

        Semicolon = 1 << 5,
        Colon = 1 << 6,
        Assignment = 1 << 7,
        Comma = 1 << 8,

        Parentheses = 1 << 9,
        SquareBrackets = 1 << 10,
        AngleBrackets = 1 << 11,
        Scope = 1 << 12,

        Start = 1 << 13,
        End = 1 << 14,

        Name = 1 << 15,
        Body = 1 << 16,

        LineDirective = 1 << 17 | Directive,

        StartContainer = Start | Container,
        EndContainer = End | Container,

        DirectiveName = Name | Directive,
        DirectiveBody = Body | Directive,

        LineDirectiveName = DirectiveName | LineDirective,
        LineDirectiveBody = DirectiveBody | LineDirective,

        SemicolonSeparator = Semicolon | Separator,
        ColonSeparator = Colon | Separator,
        AssignmentSeparator = Assignment | Separator,
        CommaSeparator = Comma | Separator,

        ParenthesesPreamble = Parentheses | Preamble,
        SquareBracketsPreamble = SquareBrackets | Preamble,
        AngleBracketsPreamble = AngleBrackets | Preamble,
        ScopePreamble = Scope | Preamble,

        StartScope = Scope | StartContainer,
        EndScope = Scope | EndContainer,

        OpenParentheses = Parentheses | StartContainer,
        CloseParentheses = Parentheses | EndContainer,

        OpenSquareBrackets = SquareBrackets | StartContainer,
        CloseSquareBrackets = SquareBrackets | EndContainer,

        OpenAngleBrackets = AngleBrackets | StartContainer,
        CloseAngleBrackets = AngleBrackets | EndContainer
    };

    /// <summary>
    /// Source file associated with LexBlocks
    /// </summary>
    struct LexFile
    {
        string_view filePath;
        int startLine;
    };

    /// <summary>
    /// A range of source representing a group of lexemes, bounded by punctuation
    /// </summary>
    struct LexBlock
    {
        TextBlock src;
        LexBlockTypes type;
        int file;
        int depth;
        int startLine;
        int lineCount;

        LexBlock() :
            file(0),
            depth(-1),
            startLine(0),
            lineCount(0),
            type(LexBlockTypes::Unknown)
        { }

        LexBlock(int depth, LexBlockTypes type, char* pStart, int startLine = 0, int lineBreaks = 0, int file = 0) :
            file(file),
            depth(depth),
            startLine(startLine),
            lineCount(lineBreaks),
            type(type),
            src(pStart)
        { }

        LexBlock(int depth, LexBlockTypes type, char* pStart, char* pEnd, int startLine = 0, int lineBreaks = 0, int file = 0) :
            file(file),
            depth(depth),
            startLine(startLine),
            lineCount(lineBreaks),
            type(type),
            src(pStart, pEnd)
        { }

        LexBlock(int depth, LexBlockTypes type, TextBlock src, int startLine = 0, int lineBreaks = 0, int file = 0) :
            file(file),
            depth(depth),
            startLine(startLine),
            lineCount(lineBreaks),
            type(type),
            src(src)
        { }

        bool GetHasAnyFlag(LexBlockTypes flag) const { return (uint)(type & flag) > 0; }

        bool GetHasFlags(LexBlockTypes flag) const { return (type & flag) == flag; }

        int GetLastLine() const { return startLine + lineCount; }
    };

    /// <summary>
    /// Decomposes pre-sanitized source into contiguous chunks represented by LexBlocks
    /// </summary>
    class BlockAnalyzer
    {
    public:
        MAKE_MOVE_ONLY(BlockAnalyzer)

        BlockAnalyzer();

        void Clear();

        void AnalyzeSource(string_view path, TextBlock src);

        const IDynamicArray<LexFile>& GetSourceFiles() const;

        const IDynamicArray<LexBlock>& GetBlocks() const;

    private:
        TextBlock src;
        UniqueVector<LexFile> files;
        UniqueVector<LexBlock> blocks;
        UniqueVector<int> containers;
        const char* pPosOld;

        const char* pPos;
        int depth;
        int line;

        void AddBlock(const TextBlock& start);

        LexBlock* GetTopContainer();

        void StartContainer();

        void EndContainer();

        void AddDirective();

        void ProcessLineDirective(LexBlock& body);

        void SetState(int blockIndex);

        void RevertContainer(int blockIndex);

        void RevertTemplate();

        bool GetCanOpenTemplate() const;

        bool GetCanCloseTemplate() const;

        string_view GetBreakFilter() const;

        bool TryFinalizeParse();

        int GetFileIndex() const;
    };
}
