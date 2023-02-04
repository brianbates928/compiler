/* Implementation of PLSL Interpreter
 * parseInt.cpp
 * Programming Assignment 3
 * Spring 2022
*/

#include <vector>
#include <string>
#include "parseInt.h"
#include "val.h"
//#include "lex.cpp"
using namespace std;
map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {
    bool pushed_back = false;
    LexItem    pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
    ++error_count;
    cout << line << ": " << msg << endl;
}

/*
    //Pascal-Like Small Language (PLSL)
    //1. Prog ::= PROGRAM IDENT; DeclBlock ProgBody
    //2. DeclBlock ::= VAR {DeclStmt;}
    //3. DeclStmt ::= Ident {, Ident} : (Integer | Real | String)
    //4. ProgBody ::= BEGIN {Stmt;} END
    //5. Stmt ::= AssigStmt | IfStmt | WriteLnStmt | ForStmt
    //6. WriteLnStmt ::= WRITELN (ExprList)
    //7. IfStmt ::= IF ( LogicExpr ) THEN Stmt [ELSE Stmt]
    //8. ForStmt ::= FOR Var := ICONST (TO | DOWNTO) ICONST DO Stmt
    //9. AssignStmt ::= Var := Expr
    //10. ExprList ::= Expr {, Expr}
    //11. Expr ::= Term {(+|-) Term}
    //12. Term ::= SFactor {( * | / ) SFactor}
    //13. SFactor ::= [(+ | -)] Factor
    //14. LogicExpr ::= Expr (= | > | <) Expr
    //15. Var ::= IDENT
    //16. Factor ::= IDENT | ICONST | RCONST | SCONST | (Expr)
    //bool IdentList(istream& in, int& line, vector<string> &IdList);
*/

ostream& operator<<(ostream& out, const map<string,Value>& op) {
    for(auto i=op.begin();i!=op.end();i++){
        out<<i->first<<": "<<i->second<<endl;
    }
    return out;
}

//Stmt is either a WriteLnStmt, ForepeatStmt, IfStmt, or AssigStmt
//Stmt = AssigStmt | IfStmt | WriteStmt | ForStmt
bool Stmt(istream& in, int& line) {
    bool status;
    //cout << "in ContrlStmt" << endl;
    LexItem t = Parser::GetNextToken(in, line);
    
    switch( t.GetToken() ) {

    case WRITELN:
        status = WriteLnStmt(in, line);
        //cout << "After WriteStmet status: " << (status? true:false) <<endl;
        break;

    case IF:
        status = IfStmt(in, line);
        break;

    case IDENT:
        Parser::PushBackToken(t);
        status = AssignStmt(in, line);
        
        break;
        
    case FOR:
        //status = ForStmt(in, line);
        
        break;
        
        
    default:
        Parser::PushBackToken(t);
        return false;
    }

    return status;
}//End of Stmt


//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
    bool status = false;
    //cout << "in ExprList and before calling Expr" << endl;
    
    Value givenval1;
    status = Expr(in, line, givenval1);
    if(!status){
        ParseError(line, "Missing Expression");
        return false;
    }
    
    (*ValQue).push(givenval1);
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok == COMMA) {
        //cout << "before calling ExprList" << endl;
        status = ExprList(in, line);
        //cout << "after calling ExprList" << endl;
    }
    else if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    else{
        Parser::PushBackToken(tok);
        return true;
    }
    return status;
}

//Bool for Prog
bool Prog(istream& in, int& line) {
    LexItem progcheck = Parser::GetNextToken(in, line);
    if (progcheck != PROGRAM) {
        ParseError(line, "Missing PROGRAM.");
        return false;
    }

    LexItem identcheck = Parser::GetNextToken(in,line);
    if (identcheck != IDENT) {
        ParseError(line,"Missing Program Name.");
        return false;
    }
    
    //Put the ident into the map
    defVar[identcheck.GetLexeme()]=true;
    
    LexItem semicoloncheck = Parser::GetNextToken(in,line);
    if (semicoloncheck != SEMICOL) {
        ParseError(line,"Missing semicolon in Statement.");
        return false;
    }
    
    bool declblockcheck = DeclBlock(in,line);
    if (!declblockcheck) {
        ParseError(line,"Incorrect Delcaration Section.");
        return false;
    }
    
    bool progbodycheck = ProgBody(in,line);
    if (!progbodycheck) {
        ParseError(line,"Incorrect Program Body.");
        return false;
    }
    
    return true;
}

//Bool for Declblock
bool DeclBlock(istream& in, int& line) {
    LexItem varcheck = Parser::GetNextToken(in,line);
    if (varcheck != VAR) {
        ParseError(line,"Non-recognizable Declaration Block.");
        return false;
    }
    
    while (true) {
        LexItem declendcheck = Parser::GetNextToken(in,line);
        Parser::PushBackToken(declendcheck);
        if (declendcheck == BEGIN) break;
        
        bool declstmtcheck = DeclStmt(in,line);
        if (!declstmtcheck) {
            ParseError(line,"Syntactic error in Declaration Block.");
            return false;
        }
        
        LexItem semicoloncheck = Parser::GetNextToken(in,line);
        if (semicoloncheck != SEMICOL) {
            ParseError(line,"Missing SEMICOL following DeclStmt");
            return false;
        }
    }
    return true;
}

//Bool for DeclStmt
bool DeclStmt(istream& in, int& line) {
    LexItem identcheck;
    LexItem commacheck;
    vector<string> idents;
        while (true) {
            identcheck = Parser::GetNextToken(in,line);
            if (identcheck != IDENT) {
                ParseError(line,"Missing 'IDENT' in DeclStmt");
                return false;
            }
            
            //Check for Reclaration
            if ((defVar.find(identcheck.GetLexeme())->second) == true){
                ParseError(line, "Variable Redefinition");
                ParseError(line, "Incorrect variable in Declaration Statement.");
                return false;
            }
            defVar[identcheck.GetLexeme()]=true;
            idents.push_back(identcheck.GetLexeme());
            
            commacheck = Parser::GetNextToken(in,line);
            
            if (commacheck != COMMA) {
                Parser::PushBackToken(commacheck);
                break;
            }
        }

        LexItem coloncheck = Parser::GetNextToken(in,line);
        if (coloncheck != COLON) {
            ParseError(line,"Missing COLON in DelcStmt");
            return false;
        }
        
        LexItem declcheck = Parser::GetNextToken(in,line);
        if (declcheck != INTEGER && declcheck != REAL && declcheck != STRING) {
            ParseError(line,"Incorrect Declaration Type.");
            return false;
        }
    for (auto i = idents.begin(); i != idents.end(); i++)
            SymTable[*i]=declcheck.GetToken();
    return true;
}

//Bool for Progbody
bool ProgBody(istream& in, int& line) {
    LexItem begincheck = Parser::GetNextToken(in,line);
    if (begincheck != BEGIN) {
        ParseError(line,"Missing BEGIN");
        return false;
    }
    
    while (true) {
        LexItem endcheck = Parser::GetNextToken(in,line);
        if (endcheck == END) break;
        Parser::PushBackToken(endcheck);
        int oldlinenumber = line;
        
        bool stmtcheck = Stmt(in,line);
        if (stmtcheck) {
            LexItem semicoloncheck = Parser::GetNextToken(in,line);
            if (semicoloncheck != SEMICOL) {
                if (line != oldlinenumber) {
                    line = oldlinenumber;
                }
                ParseError(line,"Unrecognized Input Pattern");
                return false;
            }
        }
        else {
            ParseError(line,"Syntactic error in Program Body");
            return false;
        }
    }
    return true;
}

//Bool for IfStmt
bool IfStmt(istream& in, int& line) {
    //Don't need this
//    LexItem ifcheck = Parser::GetNextToken(in,line);
//    if (ifcheck != IF) {
//        ParseError(line,"Missing IF");
//        return false;
//    }
    LexItem leftparcheck = Parser::GetNextToken(in,line);
    if (leftparcheck != LPAREN) {
        ParseError(line,"Missing '('");
        return false;
    }
    
    Value givenval1;
    bool logicexprcheck = LogicExpr(in,line,givenval1);
    if (!logicexprcheck ||givenval1.IsErr()) {
        ParseError(line, "Missing LogicExpr in IfStmt!");
        return false;
    }
    
    LexItem rightparcheck = Parser::GetNextToken(in,line);
    if (rightparcheck != RPAREN) {
        ParseError(line, "Missing ')'");
        return false;
    }
    
    LexItem thencheck = Parser::GetNextToken(in,line);
    if (thencheck != THEN) {
        ParseError(line, "Missing Then in IfStmt");
        return false;
    }
    
    if (givenval1.GetBool()) {
        bool stmtcheck = Stmt(in,line);
        if (!stmtcheck) {
            ParseError(line,"Missing Stmt after Then in IfStmt");
            return false;
        }
    }else{
        LexItem thing=Parser::GetNextToken(in, line);;
        while(thing!=ELSE && thing!=SEMICOL){
            if(thing==ERR ||thing==DONE){
                ParseError(line,"Error If stmt");
                return false;
            }
            thing=Parser::GetNextToken(in, line);;
        }
        Parser::PushBackToken(thing);
    }
    
    LexItem elsecheck = Parser::GetNextToken(in,line);
    if (elsecheck == ELSE) {
        if(!givenval1.GetBool()){
            bool stmtcheck = Stmt(in,line);
            if (!stmtcheck) {
                ParseError(line,"Missing Stmt after ELSE in IfStmt");
                return false;
            }
        }else{
            LexItem thing=Parser::GetNextToken(in, line);;
            while(thing!=ELSE && thing!=SEMICOL){
                if(thing==ERR || thing==DONE){
                    ParseError(line,"Error If stmt");
                    return false;
                }
                thing=Parser::GetNextToken(in, line);;
            }
            Parser::PushBackToken(thing);
        }
    }
    else Parser::PushBackToken(elsecheck);
    
    return true;
}


//Bool for AssignStmt
bool AssignStmt(istream& in, int& line) {
    
    LexItem giventok1;
    bool varcheck = Var(in,line,giventok1);
    if (!varcheck) {
        ParseError(line,"Missing Var in AssignStmt");
        return false;
    }
    
    LexItem assopcheck = Parser::GetNextToken(in,line);
    if (assopcheck != ASSOP) {
        ParseError(line,"Missing ASSOP in the AssignStmt");
        return false;
    }
    
    Value givenval1;
    bool exprcheck = Expr(in,line,givenval1);
    if (!exprcheck) {
        ParseError(line,"Missing Expr in the AssignStmt");
        return false;
    }
    if (SymTable[giventok1.GetLexeme()] == REAL) {
        if (givenval1.GetType()==VREAL){}
        else if (givenval1.GetType() == VINT) {
            givenval1.SetReal(givenval1.GetInt());
            givenval1.SetType(VREAL);
        }else{
            ParseError(line,"Illegal Assignment Operation");
            return false;
        }
    }
    if (SymTable[giventok1.GetLexeme()] == INTEGER) {
        if (givenval1.GetType() == VREAL){
            givenval1.SetInt(givenval1.GetReal());
            givenval1.SetType(VINT);
        }
    }
    
    //Put into Map
    TempsResults[giventok1.GetLexeme()] = givenval1;
    return true;
}


//Bool for Term
bool Term(istream& in, int& line, Value & retVal) {
    
    Value givenval1, givenval2;
    bool sfactorcheck = SFactor(in,line,givenval1);
    if (!sfactorcheck) {
        //ParseError(line,"Missing starting SFactor in Term");
        return false;
    }
    
    retVal = givenval1;
    
    while (true) {
        LexItem multordivcheck = Parser::GetNextToken(in,line);
        if (multordivcheck != MULT && multordivcheck != DIV) {
            Parser::PushBackToken(multordivcheck);
            break;
        }
        
        sfactorcheck = SFactor(in,line,givenval2);
        if (!sfactorcheck) {
            //ParseError(line,"Missing SFactor after '* or /' in Term");
            return false;
        }
        
        if (multordivcheck == MULT) retVal = retVal * givenval2;
        if (multordivcheck == DIV) {
            if(givenval2.IsInt()){
                if(givenval2.GetInt()==0){
                    ParseError(line,"Run-Time Error-Illegal Division by Zero");
                    return false;
                }
            }
            if(givenval2.IsReal()){
                if(givenval2.GetReal()==0.00){
                    ParseError(line,"Run-Time Error-Illegal Division by Zero");
                    return false;
                }
            }
            retVal = retVal / givenval2;
        }
    }
    
    return true;
}

//Bool for SFactor
bool SFactor(istream& in, int& line, Value & retVal) {
    int sign = 0;
    LexItem plusorminuscheck = Parser::GetNextToken(in,line);
    if (plusorminuscheck == PLUS) sign = 1;
    else if (plusorminuscheck == MINUS) sign = -1;
    else Parser::PushBackToken(plusorminuscheck);
    
    Value givenval1;
    bool factorcheck = Factor(in,line,sign,givenval1);
    if(sign!=0 && givenval1.IsString()){
        ParseError(line,"Illegal Operand Type for Sign Operator");
        return false;
    }
    if (!factorcheck) {
        //ParseError(line,"Missing Factor in SFactor");
        return false;
    }
    retVal = givenval1;
    return true;
}

//Bool for LogicExpr
bool LogicExpr(istream& in, int& line, Value & retVal) {
    Value givenval,givenval2;
    bool exprcheck = Expr(in,line,givenval);
    if (!exprcheck) {
        ParseError(line,"Missing starting Expr in LogicExpr");
        return false;
    }
    
    LexItem equalorgthanorlthancheck = Parser::GetNextToken(in,line);
    if (equalorgthanorlthancheck != EQUAL && equalorgthanorlthancheck != GTHAN && equalorgthanorlthancheck != LTHAN) {
        ParseError(line,"Missing '= or < or >' in LogicExpr");
        return false;
    }
    
    exprcheck = Expr(in,line,givenval2);
    if (!exprcheck) {
        ParseError(line,"Missing ending Expr in LogicExpr");
        return false;
    }

    if(equalorgthanorlthancheck==EQUAL){
        retVal= givenval==givenval2;
    }else if(equalorgthanorlthancheck==LTHAN){
        retVal= givenval<givenval2;
    }else if (equalorgthanorlthancheck==GTHAN) {
        retVal= givenval>givenval2;
    }

    if(retVal.IsErr()){
        ParseError(line,"Run-Time Error-Illegal Mixed Type Operands for a Logic Expression");
        return false;
    }
    //cout<<retVal<<endl<<line<<": line   "<<givenval<<" : "<<givenval2<<endl;
    return true;
}

//Bool for Var
bool Var(istream& in, int& line, LexItem & idtok) {
    LexItem identcheck = Parser::GetNextToken(in,line);
    idtok=identcheck;
    if (identcheck != IDENT) {
        ParseError(line,"Missing IDENT in Var");
        return false;
    }
    
    if((defVar.find(identcheck.GetLexeme())->second) == false) {
        ParseError(line,"Undeclared Variable");
        return false;
    }
    
    return true;
}

//Bool for Factor
bool Factor(istream& in, int& line, int sign, Value & retVal) {
    LexItem itemcheck = Parser::GetNextToken(in,line);
    if (itemcheck == ICONST || itemcheck == RCONST || itemcheck == SCONST) {
        if (itemcheck == ICONST) {
            retVal = Value (std::stoi(itemcheck.GetLexeme()));
            if(sign==-1)
                retVal.SetInt(-1*retVal.GetInt());
        }
        else if (itemcheck == RCONST) {
            retVal = Value (std::stof(itemcheck.GetLexeme()));
            if(sign==-1)
                retVal.SetReal(retVal.GetReal()*-1);
        }else{
            retVal = Value (itemcheck.GetLexeme());
        }
        return true;
    }
    
    if (itemcheck == IDENT) {
        if (defVar.find(itemcheck.GetLexeme()) == defVar.end()){
            ParseError(line,"Using Undefined Variable");
            return false;
        }
        
        if (TempsResults.find(itemcheck.GetLexeme()) == TempsResults.end()) {
            ParseError(line, "Undefined Variable: " + itemcheck.GetLexeme());
            return false;
        }
        retVal = TempsResults[itemcheck.GetLexeme()];
        if (sign==-1){
            if(retVal.GetType()==VINT)
                retVal.SetInt(retVal.GetInt()*-1);
            if(retVal.GetType()==VREAL)
                retVal.SetReal(retVal.GetReal()*-1);
        }
        return true;
    }
    
    if (itemcheck != LPAREN) return false;
    
    Value givenval1;
    bool exprcheck = Expr(in,line,givenval1);
    if (!exprcheck) {
        ParseError(line,"Missing Expr after LPAREN in Factor");
        return false;
    }
    
    itemcheck = Parser::GetNextToken(in,line);
    if (itemcheck != RPAREN) {
        ParseError(line,"Missing RPAREN after Expr in Factor");
        return false;
    }
    retVal = givenval1;
    return true;
}





//WriteStmt:= wi, ExpreList
bool WriteLnStmt(istream& in, int& line) {
    LexItem t;
    ValQue = new queue<Value>;
    t = Parser::GetNextToken(in, line);
    if( t != LPAREN ) {
        
        ParseError(line, "Missing Left Parenthesis");
        return false;
    }
    
    bool ex = ExprList(in, line);
    
    if( !ex ) {
        ParseError(line, "Missing expression after WriteLn");
        return false;
    }
    
    t = Parser::GetNextToken(in, line);
    if(t != RPAREN ) {
        
        ParseError(line, "Missing Right Parenthesis");
        return false;
    }
    
    
    //Evaluate: print out the list of expressions' values
    while (!(*ValQue).empty())
    {
        Value nextVal = (*ValQue).front();
        cout << nextVal;
        ValQue->pop();
    }
    cout << endl;

    return ex;
}//End of WriteLnStmt


//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value & retVal) {
    Value val1, val2;
    bool t1 = Term(in, line, val1);
    LexItem tok;
    
    if( !t1 ) {
        return false;
    }
    retVal = val1;
    
    tok = Parser::GetNextToken(in, line);
    if(tok.GetToken() == ERR){
        ParseError(line, "Unrecognized Input Pattern");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    }
    //Evaluate: evaluate the expression for addition or subtraction
    while ( tok == PLUS || tok == MINUS )
    {
        t1 = Term(in, line, val2);
        if( !t1 )
        {
            ParseError(line, "Missing operand after operator");
            return false;
        }
        
        if(tok == PLUS)
        {
            retVal = retVal + val2;
            if(retVal.IsErr())
            {
                ParseError(line, "Illegal addition operation.");
                return false;
            }
        }
        else if(tok == MINUS)
        {
            retVal = retVal - val2;
            if(retVal.IsErr())
            {
                ParseError(line, "Illegal subtraction operation.");
                return false;
            }
        }
            
        tok = Parser::GetNextToken(in, line);
        if(tok.GetToken() == ERR){
            ParseError(line, "Unrecognized Input Pattern");
            cout << "(" << tok.GetLexeme() << ")" << endl;
            return false;
        }
        
        
    }
    Parser::PushBackToken(tok);
    return true;
}//end of Expr





