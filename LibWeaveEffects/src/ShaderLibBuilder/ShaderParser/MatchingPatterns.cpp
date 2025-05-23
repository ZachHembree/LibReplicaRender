#include "pch.hpp"
#include "WeaveEffects/ShaderLibBuilder/ShaderParser/SymbolPatterns.hpp"

using namespace Weave;
using namespace Weave::Effects;

static const MatchNode s_AttributePattern = 
{
    {
        MatchPattern { LexBlockTypes::OpenSquareBrackets },
        MatchNode 
        {
            {
                MatchNode // Attrib with args
                {
                    MatchPattern
                    {
                        LexBlockTypes::ParenthesesPreamble,
                        CapturePattern { TokenTypes::AttribIdent }
                    },
                    { LexBlockTypes::OpenParentheses },
                    MatchPattern
                    {
                        { LexBlockTypes::CommaSeparator, LexBlockTypes::Unterminated },
                        CapturePattern { TokenTypes::LiteralArg },
                        MatchQualifiers::OneOrMore | MatchQualifiers::Alternation
                    },
                    { LexBlockTypes::CloseParentheses },
                },
                MatchPattern // Attrib without args
                {
                    LexBlockTypes::Unterminated,
                    CapturePattern { TokenTypes::AttribIdent }
                }
            },        
            MatchQualifiers::Alternation
        },
        MatchPattern { LexBlockTypes::CloseSquareBrackets }
    },
    TokenTypes::AttribIdent,
    MatchQualifiers::ZeroOrMore
};

static const CapturePattern s_TypeCapPattern = 
{ 
    {{ TokenTypes::TypeModifier, MatchQualifiers::ZeroOrMore }}, 
    TokenTypes::Type 
};

static MatchNode GetTemplatedTypePattern(const CapturePattern& cap)
{
    return 
    {
        {
            MatchPattern
            {
                LexBlockTypes::AngleBracketsPreamble,
                cap
            },
            MatchPattern { LexBlockTypes::OpenAngleBrackets },
            MatchPattern // Template args
            {
                { LexBlockTypes::CommaSeparator, LexBlockTypes::Unterminated },
                CapturePattern { TokenTypes::LiteralArg },
                MatchQualifiers::OneOrMore | MatchQualifiers::Alternation,
            },
            MatchPattern { LexBlockTypes::CloseAngleBrackets }
        },
        TokenTypes::Type
    };
}

static const MatchNode s_TemplatedTypePattern = GetTemplatedTypePattern(
    { {{ TokenTypes::TypeModifier, MatchQualifiers::ZeroOrMore }}, TokenTypes::Type }
);

static MatchNode GetParamPattern(const CapturePattern& typeCap = {})
{
    return MatchNode
    {
        {
            MatchNode // Without semantic
            {
                MatchPattern
                {
                    { LexBlockTypes::CommaSeparator, LexBlockTypes::Unterminated },
                    {
                        typeCap,
                        CapturePattern { TokenTypes::ParamIdent, SymbolTypes::Parameter },
                    },
                    MatchQualifiers::Alternation
                },
            },
            MatchNode // With semantic
            {
                MatchPattern
                {
                    { LexBlockTypes::ColonSeparator },
                    {
                        typeCap,
                        CapturePattern { TokenTypes::ParamIdent, SymbolTypes::Parameter },
                    },
                },
                MatchPattern
                {
                    { LexBlockTypes::CommaSeparator, LexBlockTypes::Unterminated },
                    CapturePattern { TokenTypes::SemanticIdent },
                    MatchQualifiers::Alternation
                },
            },
        },
        MatchQualifiers::Alternation
    };
}

static const MatchNode s_ParamsPattern =
{
    {
        s_AttributePattern,
        MatchNode
        {
            {
                MatchNode // With templated type
                {
                    s_TemplatedTypePattern,
                    GetParamPattern(),
                },
                // Without templated type
                GetParamPattern(s_TypeCapPattern),
            },
            MatchQualifiers::Alternation
        },
    },
    TokenTypes::ParamIdent,
    MatchQualifiers::ZeroOrMore
};

static const MatchNode s_ReturnSemantic =
{
    {
        MatchPattern { LexBlockTypes::ColonSeparator },
        MatchPattern 
        { 
            LexBlockTypes::ScopePreamble,
            CapturePattern { TokenTypes::SemanticIdent }
        }
    },
    MatchQualifiers::Optional,
};

static MatchNode GetNamedScopePattern(const std::initializer_list<TokenQualifier>& predicates, TokenTypes ident, SymbolTypes symbol) noexcept
{
    return MatchNode
    {
        {
            s_AttributePattern,
            MatchNode
            {
                MatchPattern
                {
                    { LexBlockTypes::ScopePreamble },
                    { predicates, ident, symbol },
                },
                MatchQualifiers::None,
                true
            }
        },
        ident
    };
}

static MatchNode GetShaderPatterns(const std::initializer_list<TokenQualifier>& predicates, TokenTypes ident, SymbolTypes symbol) noexcept
{
    return MatchNode
    {
        {
            // Scoped definition
            GetNamedScopePattern(predicates, ident, SymbolTypes::WeaveDefinition | symbol),
            // Effect pass assignment
            MatchNode
            {
                {
                    MatchPattern { LexBlockTypes::AssignmentSeparator },
                    MatchPattern
                    {
                        { LexBlockTypes::SemicolonSeparator },
                        CapturePattern { TokenTypes::Technique | ident, SymbolTypes::TechniqueShaderDecl | (symbol & SymbolTypes::ShaderMask)}
                    }
                }
            }
        },
        MatchQualifiers::Alternation
    };
}

static MatchNode GetVarPatterns(const CapturePattern& typeCap = {})
{
    return MatchNode 
    {
        {
            MatchNode // Normal var w/o init
            {
                {
                    MatchPattern  // Without semantic
                    {
                        { LexBlockTypes::SemicolonSeparator },
                        {
                            typeCap,
                            CapturePattern { TokenTypes::VarIdent, SymbolTypes::VariableDecl }
                        }
                    },
                    MatchNode // With semantic
                    {
                        MatchPattern
                        {
                            { LexBlockTypes::ColonSeparator },
                            {
                                typeCap,
                                CapturePattern { TokenTypes::VarIdent, SymbolTypes::VariableDecl }
                            }
                        },
                        MatchPattern
                        {
                            { LexBlockTypes::SemicolonSeparator },
                            CapturePattern { TokenTypes::SemanticIdent }
                        },
                    },
                },
                MatchQualifiers::Alternation
            },
            MatchNode // Array
            {
                {
                    MatchPattern
                    {
                        {
                            LexBlockTypes::SquareBracketsPreamble,
                            LexBlockTypes::OpenSquareBrackets,
                            { BlockQualifier::Wild, MatchQualifiers::ZeroOrMore },
                            LexBlockTypes::CloseSquareBrackets
                        },
                        {
                            typeCap,
                            CapturePattern { TokenTypes::VarIdent, SymbolTypes::VariableDecl },
                        },
                    },
                    MatchNode
                    {
                        {
                            MatchNode // with semantic
                            {
                                MatchPattern { LexBlockTypes::ColonSeparator },
                                MatchPattern
                                {
                                    { LexBlockTypes::SemicolonSeparator },
                                    CapturePattern { TokenTypes::SemanticIdent }
                                }
                            },
                            // without semantic
                            MatchPattern { LexBlockTypes::SemicolonSeparator }
                        },
                        MatchQualifiers::Alternation
                    }
                },
            },
            MatchNode // Var assign init
            {
                {
                    MatchPattern
                    {
                        {
                            LexBlockTypes::AssignmentSeparator ,
                            { BlockQualifier::Wild, MatchQualifiers::OneOrMore },
                            LexBlockTypes::SemicolonSeparator
                        },
                        {
                            typeCap,
                            CapturePattern { TokenTypes::VarIdent, SymbolTypes::VariableAssignDef }
                        },
                    }
                },
            },
            MatchNode // Ambiguous func or var init
            {
                {
                    MatchPattern
                    {
                        {
                            LexBlockTypes::ParenthesesPreamble,
                            // Params or args, no capturing
                            LexBlockTypes::OpenParentheses,
                            { BlockQualifier::Wild, MatchQualifiers::ZeroOrMore },
                            LexBlockTypes::CloseParentheses,
                            LexBlockTypes::SemicolonSeparator
                        },
                    },
                },
            },
        },
        MatchQualifiers::Alternation
    };
}

const UniqueArray<MatchNodeGroup> MatchNodeGroup::MatchNodeGroups =
{
    // Typedef
    MatchNodeGroup
    {
        { TokenTypes::TypedefDecl },
        {           
            MatchNode 
            {
                MatchPattern // Normal base type
                {
                    { LexBlockTypes::SemicolonSeparator },
                    {
                        CapturePattern { { TokenTypes::TypedefDecl, { TokenTypes::TypeModifier, MatchQualifiers::ZeroOrMore } }, TokenTypes::Type },
                        CapturePattern { TokenTypes::TypeAlias, SymbolTypes::TypedefDecl }
                    },
                },
                TokenTypes::TypeAlias
            },
            MatchNode // Templated base type
            {
                {
                    GetTemplatedTypePattern
                    (
                        {{ TokenTypes::TypedefDecl, { TokenTypes::TypeModifier, MatchQualifiers::ZeroOrMore } }, TokenTypes::Type }
                    ),
                    MatchPattern // Alias identifier
                    {
                        { LexBlockTypes::SemicolonSeparator },
                        CapturePattern { TokenTypes::TypeAlias, SymbolTypes::TypedefDecl }
                    }
                },
                TokenTypes::TypeAlias
            }
        }
    },
    // Struct
    MatchNodeGroup
    {
        { TokenTypes::StructDecl },
        {
            {
                // Struct definition
                GetNamedScopePattern({ TokenTypes::StructDecl }, TokenTypes::StructIdent, SymbolTypes::StructDef),
                // Struct declaration
            }
        }
    },
    // Constant buffers
    MatchNodeGroup
    {
        { TokenTypes::ConstBufDecl },
        { GetNamedScopePattern({ TokenTypes::ConstBufDecl }, TokenTypes::ConstBufIdent, SymbolTypes::ConstBufDef) }
    },
    // Weave blocks
    // Vertex
    MatchNodeGroup
    {
        { TokenTypes::VertexShaderDecl },
        { GetShaderPatterns({ TokenTypes::VertexShaderDecl }, TokenTypes::VertexIdent, SymbolTypes::VertexShaderDef) }
    },
    // Hull
    MatchNodeGroup
    {
        { TokenTypes::HullShaderDecl },
        { GetShaderPatterns({ TokenTypes::HullShaderDecl }, TokenTypes::HullIdent, SymbolTypes::HullShaderDef) }
    },
    // Domain
    MatchNodeGroup
    {
        { TokenTypes::DomainShaderDecl },
        { GetShaderPatterns({ TokenTypes::DomainShaderDecl }, TokenTypes::DomainIdent, SymbolTypes::DomainShaderDef) }
    },
    // Geometry
    MatchNodeGroup
    {
        { TokenTypes::GeometryShaderDecl },
        { GetShaderPatterns({ TokenTypes::GeometryShaderDecl }, TokenTypes::GeometryIdent, SymbolTypes::GeometryShaderDef) }
    },
    // Pixel
    MatchNodeGroup
    {
        { TokenTypes::PixelShaderDecl },
        { GetShaderPatterns({ TokenTypes::PixelShaderDecl }, TokenTypes::PixelIdent, SymbolTypes::PixelShaderDef) }
    },
    // Compute
    MatchNodeGroup
    {
        { TokenTypes::ComputeShaderDecl },
        { GetShaderPatterns({ TokenTypes::ComputeShaderDecl }, TokenTypes::ComputeIdent, SymbolTypes::ComputeShaderDef) }
    },
    // Technique
    MatchNodeGroup
    {
        { TokenTypes::TechniqueDecl },
        { GetNamedScopePattern({ TokenTypes::TechniqueDecl }, TokenTypes::TechniqueIdent, SymbolTypes::TechniqueDef) }
    },
    // Pass
    MatchNodeGroup
    {
        { TokenTypes::PassDecl },
        { GetNamedScopePattern({ TokenTypes::PassDecl }, TokenTypes::PassIdent, SymbolTypes::TechniquePassDecl) }
    },
    // Function or variable
    MatchNodeGroup
    {
        // Starting type
        { TokenTypes::Type, TokenTypes::TypeModifier },
        // Symbol definitions
        {
            // Variable declaration
            MatchNode
            {
                {
                    s_AttributePattern,
                    MatchNode // Var variants
                    {
                        { 
                            // Templated var types
                            MatchNode
                            {
                                s_TemplatedTypePattern,
                                GetVarPatterns(),
                            },
                            // Non-templated var types
                            GetVarPatterns(s_TypeCapPattern),
                        },
                        MatchQualifiers::Alternation,
                        true
                    },
                },
                TokenTypes::VarIdent
            },
            // Function definition
            MatchNode
            {
                {
                    {
                        s_AttributePattern,
                        // Identifier
                        MatchNode
                        {   
                            {
                                MatchPattern // Without templated type
                                {
                                    LexBlockTypes::ParenthesesPreamble,
                                    {
                                        s_TypeCapPattern,
                                        CapturePattern { TokenTypes::FuncIdent, SymbolTypes::FuncDefinition },
                                    },
                                },
                                MatchNode // Templated type
                                {
                                    {
                                        s_TemplatedTypePattern,
                                        MatchPattern
                                        {
                                            LexBlockTypes::ParenthesesPreamble,
                                            CapturePattern { TokenTypes::FuncIdent, SymbolTypes::FuncDefinition },
                                        }
                                    }
                                },
                            },
                            MatchQualifiers::Alternation,
                            true
                        },
                        // Params
                        MatchPattern { LexBlockTypes::OpenParentheses },
                        s_ParamsPattern,
                        MatchPattern { LexBlockTypes::CloseParentheses },
                        s_ReturnSemantic,
                    },
                    TokenTypes::FuncIdent
                },
                MatchPattern { LexBlockTypes::StartScope }
            }
        }
    }
};