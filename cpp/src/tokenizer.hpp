#ifndef __HAVE_PARSOID_TOKENIZER_H__
#define __HAVE_PARSOID_TOKENIZER_H__

#include <vector>
#include "parsoid_internal.hpp"

namespace parsoid {
    using std::string;
    using std::vector;

    class WikiTokenizer {
        public:
            WikiTokenizer( const string& input );
            int tokenize();

            // Accumulator interface
            void emit(Tk tk) {
                return accumStack.push(tk);
            }
            TokenChunkPtr pushScope() {
                return accumStack.pushScope();
            }
            TokenChunkPtr popScope() {
                return accumStack.popScope();
            }
            TokenChunkPtr getAccum() {
                return accumStack.get();
            }

            /** 
             * Token accumulator stack
             * Supports the nested accumulation of tokens, which is needed for attributes
             * and other encapsulated bits of content (inline etc).
             */
            class AccumStack {
                public:
                    AccumStack() {
                        accumStack.push_back( TokenChunkPtr(new TokenChunk()) );
                        curAccum = accumStack.back();
                    }

                    void push( Tk tk ) {
                        return curAccum->push_back( tk );
                    }

                    TokenChunkPtr pushScope( ) {
                        accumStack.push_back( mkTokenChunk() ); // starts a new nested accum
                        curAccum = accumStack.back();
                        return curAccum;
                    }
                        
                    TokenChunkPtr popScope() {
                        TokenChunkPtr tk = accumStack.back();
                        accumStack.pop_back();
                        curAccum = accumStack.back();
                        return tk;
                    }
                    
                    TokenChunkPtr get() {
                        return curAccum;
                    }
                private:
                    vector<TokenChunkPtr> accumStack;
                    TokenChunkPtr curAccum;
            };

            /**
             * Syntactic flags are used to express syntactical restrictions in nested
             * content. An example would be 'inline, but no nested links'. We could also
             * 'unroll' this by defining individual sets of productions for the various
             * parsing contexts at the cost of duplication of code.
             */
            class SyntaxFlags {
                public:
                    enum class Flag {
                        Equal = 0,
                        Table,
                        Template,
                        LinkDesc,
                        Pipe,
                        TableCellArg,
                        Colon,
                        ExtLink,
                        Pre,
                        NoInclude,
                        IncludeOnly,
                        OnlyInclude
                    };

                    void push ( Flag name, int val ) {
                        flags[int(name)].push_back( val );
                    }
                    int pop ( Flag name ) {
                        int val = flags[int(name)].back();
                        flags[int(name)].pop_back();
                        return val;
                    }
                    int get( Flag name ) {
                        return flags[int(name)].back();
                    }
                    int inc ( Flag name ) {

                        return ++flags[int(name)].back();
                    }
                    int dec ( Flag name ) {
                        return --flags[int(name)].back();
                    }
                private:
                    // XXX: Can we automatically size this?
                    vector<vector<int>> flags = vector<vector<int>>(12);
            };

            // Make these public for now..
            SyntaxFlags syntaxFlags;

            ~WikiTokenizer();
        private:
            void* _ctx;
            AccumStack accumStack;
    };
}
#endif
