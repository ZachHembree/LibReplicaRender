#include "pch.hpp"
#include "ReplicaEffects/ParseExcept.hpp"
#include "ReplicaEffects/ShaderLibGen/ShaderParser/BlockAnalyzer.hpp"

namespace Replica::Effects
{
    static LexBlockTypes GetDelimiterType(const char ch)
    {
        switch (ch)
        {
        case '{':
            return LexBlockTypes::StartScope;
        case '(':
            return LexBlockTypes::OpenParentheses;
        case '[':
            return LexBlockTypes::OpenSquareBrackets;
        case '<':
            return LexBlockTypes::OpenAngleBrackets;
        case '}':
            return LexBlockTypes::EndScope;
        case ')':
            return LexBlockTypes::CloseParentheses;
        case ']':
            return LexBlockTypes::CloseSquareBrackets;
        case '>':
            return LexBlockTypes::CloseAngleBrackets;
        default:
            return LexBlockTypes::Unknown;
        }
    }

    const string_view s_BreakFilters[]
    {
        "=,:;{}()[]#", // 0 - No templates
        "=,:;{}()[]<#", // 1 - Can start templates
        "=,:;{}()[]<>#", // 2 - Can start or close templates
    };

    /// <summary>
    /// Returns appropriate filter based on current template instantiation or backtracking state.
    /// </summary>
    string_view BlockAnalyzer::GetBreakFilter() const 
    {
        const int index = (int)GetCanCloseTemplate() + (int)GetCanOpenTemplate();
        return s_BreakFilters[index];
    }

    /// <summary>
    /// Returns true if the analyzer can add a new template in its current state. If the analyzer 
    /// has not progressed past the last backtrack pass, then it cannot handle new potential 
    // template instances.
    /// </summary>
    bool BlockAnalyzer::GetCanOpenTemplate() const { return pPos > pPosOld; }

    /// <summary>
    /// Returns true if a template instantiation is pending completion and '>' should be considered.
    /// </summary>
    bool BlockAnalyzer::GetCanCloseTemplate() const { return !containers.empty() && blocks[containers.back()].GetHasFlags(LexBlockTypes::OpenAngleBrackets); }

    static bool GetHasFlags(const LexBlockTypes value, const LexBlockTypes flags) { return (value & flags) == flags; }

    BlockAnalyzer::BlockAnalyzer() :
        containers(20),
        pPos(nullptr),
        depth(0),
        line(1),
        pPosOld(nullptr)
    { }

    void BlockAnalyzer::Clear()
    {
        containers.clear();
        blocks.clear();
        depth = 0;
        line = 1;
        pPos = nullptr;
        pPosOld = nullptr;
    }

    void BlockAnalyzer::AnalyzeSource(TextBlock src)
    {
        Clear();
        this->src = src;
        pPos = src.GetFirst();

        while (true)
        {            
            // Parse
            if (pPos <= src.GetLast())
            { 
                if (*pPos <= ' ') // Skip whitespace
                {
                    if (*pPos == '\n') // Count lines
                        line++;
                }
                else // Get blocks
                {
                    switch (*pPos)
                    {
                    case '#':
                        AddDirective();
                        break;
                    case '{':
                    case '[':
                    case '(':
                        StartContainer();
                        break;
                    case '<':
                        if (GetCanCloseTemplate() || GetCanOpenTemplate())
                        {
                            StartContainer();
                            break;
                        }
                        [[fallthrough]];
                    case '}':
                    case ']':
                    case ')':
                        EndContainer();
                        break;
                    case '>':
                        if (GetCanCloseTemplate())
                        {
                            EndContainer();
                            break;
                        }
                        [[fallthrough]];
                    default:
                        AddBlock({ pPos, src.GetLast() });
                        break;
                    }
                }

                pPos++;
            }
            // Try exit
            else if (TryFinalizeParse())
                break;
        }
    }

    const IDynamicArray<LexBlock>& BlockAnalyzer::GetBlocks() const { return blocks; }

    void BlockAnalyzer::AddBlock(const TextBlock& start)
    {
        const string_view filter = GetBreakFilter();
        const char* pNext = start.GetFirst() - 1;

        do
        {
            pNext = start.FindEnd(pNext + 1, filter);
        } 
        while (pNext < start.GetLast() && TextBlock::GetIsRangeChar(*pNext, filter));

        // Create new non-container block
        LexBlock& block = blocks.emplace_back();
        
        switch (*pNext)
        {
        case '=':
            block.type = LexBlockTypes::AssignmentSeparator;
            break;
        case ';':
            block.type = LexBlockTypes::SemicolonSeparator;
            break;
        case ':':
            block.type = LexBlockTypes::ColonSeparator;
            break;
        case ',':
            block.type = LexBlockTypes::CommaSeparator;
            break;
        case '{':
            block.type = LexBlockTypes::ScopePreamble;
            break;
        case '(':
            block.type = LexBlockTypes::ParenthesesPreamble;
            break;
        case '[':
            block.type = LexBlockTypes::SquareBracketsPreamble;
            break;
        case '<':
            block.type = LexBlockTypes::AngleBracketsPreamble;
            break;
        default:
            block.type = LexBlockTypes::Unterminated;
            break;
        }

        // If unterminated, leave out last char
        if ((int)(block.type & LexBlockTypes::Separator) == 0)
            pNext--;

        block.depth = depth;
        block.src = TextBlock(start.GetFirst(), pNext);
        block.startLine = line;
        block.lineCount = block.src.FindCount('\n');

        line += block.lineCount;
        pPos = block.src.GetLast();
    }

    LexBlock* BlockAnalyzer::GetTopContainer() { return !containers.empty() ? &blocks[containers.back()] : nullptr; }

    void BlockAnalyzer::SetState(int blockIndex)
    {
        const char* pLast = pPos;

        // Restart from the beginning but without template parsing
        if (blockIndex < 0)
        {
            Clear();          
        }
        // Revert state to the point just after the given block was added and temporarily 
        // disable template parsing until this point is reached again
        else 
        { 
            PARSE_ASSERT_MSG(blockIndex >= 0 && blockIndex < (int)blocks.GetLength(), "Block index out of range")

            LexBlock& block = blocks[blockIndex];
            pPos = block.src.GetLast();
            // Depth is normally incremented after a container is added
            depth = block.depth + (int)block.GetHasFlags(LexBlockTypes::StartContainer);
            line = block.startLine + block.lineCount;

            while (!containers.empty() && containers.back() > blockIndex)
                containers.pop_back();

            blocks.RemoveRange(blockIndex + 1, (int)blocks.GetLength() - blockIndex - 1);
        }

        pPosOld = pLast;
    }

    void BlockAnalyzer::RevertContainer(int blockIndex)
    {
        PARSE_ASSERT_MSG(blockIndex >= 0 && blockIndex < (int)blocks.GetLength(), "Block index out of range")
        const LexBlock& block = blocks[blockIndex];

        // Revert to block before container preamble
        if (blockIndex > 0 && blocks[blockIndex - 1].GetHasFlags(LexBlockTypes::Preamble))
            SetState(blockIndex - 2);
        // Revert to block before container
        else
            SetState(blockIndex - 1);
    }

    void BlockAnalyzer::StartContainer()
    {
        LexBlockTypes delimType = GetDelimiterType(*pPos);

        PARSE_ASSERT_FMT(GetHasFlags(delimType, LexBlockTypes::StartContainer),
            "Unexpected expression {} on line: {}. Expected new container.", *pPos, line)

        // If an unfinished angle bracket container is on the stack, revert it if a different container
        // type is started. More restrictive than a normal parser, but should be an acceptable simplification.
        if (!GetHasFlags(delimType, LexBlockTypes::OpenAngleBrackets))
        { 
            const LexBlock* top = GetTopContainer();

            if (top != nullptr && top->GetHasFlags(LexBlockTypes::OpenAngleBrackets))
            {
                RevertContainer(containers.back());
                return;
            }
        }

        // Start new container to be later finalized in LIFO order as closing braces are encountered
        containers.push_back((int)blocks.GetLength());
        blocks.emplace_back(depth, delimType, pPos, line);
        depth++;
    }

    void BlockAnalyzer::EndContainer()
    {
        LexBlock* pCont = GetTopContainer();
        PARSE_ASSERT_FMT(pCont != nullptr, "Unexpected closing '{}' on line: {}", *pPos, line);

        // An opening '<' was previously classified as a potential template, but another container closed 
        // before it was ready. It will need to be reverted and reclassified.
        if (pCont->GetHasFlags(LexBlockTypes::OpenAngleBrackets) && (*pPos != '>'))
        {
            RevertContainer(containers.back());
        }
        // Normal bounding {}, [], () or <> containers, closed in last-in, first-out order.
        else
        { 
            LexBlockTypes delimType = GetDelimiterType(*pPos);

            if (pCont->GetHasFlags(LexBlockTypes::Scope))
            {
                PARSE_ASSERT_FMT(GetHasFlags(delimType, LexBlockTypes::EndScope), 
                    "Expected scope end '}}' on line: {}", line);
            }
            else if (pCont->GetHasFlags(LexBlockTypes::Parentheses))
            {
                PARSE_ASSERT_FMT(GetHasFlags(delimType, LexBlockTypes::CloseParentheses), 
                    "Expected closing parentheses ')' on line: {}", line);
            }
            else if (pCont->GetHasFlags(LexBlockTypes::SquareBrackets))
            {
                PARSE_ASSERT_FMT(GetHasFlags(delimType, LexBlockTypes::CloseSquareBrackets), 
                    "Expected closing square bracket ']' on line: {}", line);
            }

            depth--;
            PARSE_ASSERT_FMT(depth >= 0, "Unexpected closing '{}' on line: {}", *pPos, line)

            // Finalize source range and line count in container opening
            containers.pop_back();
            pCont->src = TextBlock(pCont->src.GetFirst(), pPos);
            pCont->lineCount = line - pCont->startLine;

            // Add duplicate ending marker with appropriate delim flags
            LexBlock& endContainer = blocks.emplace_back(*pCont);
            endContainer.type = delimType;
        }
    }

    /// <summary>
    /// Finds bounds of a single preprocessor directive starting at the given character
    /// </summary>
    void BlockAnalyzer::AddDirective()
    {
        LexBlock name, body;
        bool hasMoreLines;

        name.depth = depth;
        name.type = LexBlockTypes::DirectiveName;
        name.src = TextBlock(pPos, src.FindWordEnd(pPos) + 1);
        name.startLine = line;

        pPos = src.FindWord(name.src.GetLast() + 1);
        const char* pLast = pPos;
        int lineCount = 0;

        do
        {
            const char* lnStart = pLast + 1;
            pLast = src.Find('\n', lnStart);
            TextBlock ln(lnStart, pLast);

            hasMoreLines = ln.Contains("\\\n");
            lineCount++;

        } while (hasMoreLines);

        body.depth = depth;
        body.type = LexBlockTypes::DirectiveBody;
        body.src = TextBlock(pPos, pLast);
        body.startLine = line;
        body.lineCount = lineCount;

        line += lineCount;
        blocks.emplace_back(name);
        blocks.emplace_back(body);

        pPos = body.src.GetLast();
    }
    
    /// <summary>
    /// Returns true if parsing has completed successfully, rasies exceptions if there are unterminated 
    /// containers, and possibly backtracks to correct misidentifed trailing template instantiations.
    /// </summary>
    bool BlockAnalyzer::TryFinalizeParse()
    {
        if (!containers.empty())
        {
            LexBlock& top = *GetTopContainer();

            if (top.GetHasFlags(LexBlockTypes::StartScope))
            {
                PARSE_ERR_FMT("Unterminated scope '{{' starting on line {}", top.startLine);
            }
            else if (top.GetHasFlags(LexBlockTypes::OpenParentheses))
            {
                PARSE_ERR_FMT("Unterminated parentheses '(' starting on line {}", top.startLine);
            }
            else if (top.GetHasFlags(LexBlockTypes::OpenSquareBrackets))
            {
                PARSE_ERR_FMT("Unterminated square bracket '[' starting on line {}", top.startLine);
            }
            else if (top.GetHasFlags(LexBlockTypes::OpenAngleBrackets))
            {
                RevertContainer(containers.back());
                return false;
            }
        }

        PARSE_ASSERT_MSG(depth == 0, "Internal container parsing error.")
        return true;
    }
}