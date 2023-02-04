

/* Implementation of Recursive-Descent Parser
 * parse.cpp
 * Programming Assignment 3
 * Fall 2021
*/

#include "parseInt.h"


map<string, bool> defVar;
map<string, Token> SymTable;
map<string, Value> TempsResults; //Container of temporary locations of Value objects for results of expressions, variables values and constants 
queue <Value> * ValQue; //declare a pointer variable to a queue of Value objects

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

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




//Program is: Prog = PROGRAM IDENT {Decl} {Stmt} END PROGRAM IDENT
bool Prog(istream& in, int& line)
{
	bool dl = false;
	LexItem tok = Parser::GetNextToken(in, line);
	//cout << "in Prog" << endl;
	//cout << tok << " " << tok.GetLinenum() <<endl;
	
	if (tok.GetToken() == PROGRAM) {
		tok = Parser::GetNextToken(in, line);
		if (tok.GetToken() == IDENT) {
			dl = StmtList(in, line);
			if( !dl  )
			{
				ParseError(line, "Incorrect Syntax in the Program");
				return false;
			}
				
			tok = Parser::GetNextToken(in, line);
			
			if (tok.GetToken() == END) {
				tok = Parser::GetNextToken(in, line);
				
				if (tok.GetToken() != PROGRAM) {
					
					ParseError(line, "Missing PROGRAM at the End");
					return false;
				}	
				return true;
			}
			else
			{
				ParseError(line, "Missing END of Program");
				return false;
			}	
		}
		else
		{
			ParseError(line, "Missing Program Name.");
				return false;
		}
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if(tok.GetToken() == DONE && tok.GetLinenum() <= 1){
		ParseError(line, "Empty File");
		return false;
	}
	ParseError(line, "Missing PROGRAM.");
	return false;
}

bool StmtList(istream& in, int& line){
	bool status;
	LexItem tok;
	status = Stmt(in, line);
	//cout << "Back from stmt" << endl;
	if(status)
	{
		tok = Parser::GetNextToken(in, line);
		while(tok == SEMICOL && status)
		{
			status = Stmt(in, line);
			//cout<< "back from stmt" << endl;
			tok = Parser::GetNextToken(in, line);
		}
        
		if(tok == END && status)
		{
			Parser::PushBackToken(tok);
			return true;
		}
		else if( tok == SEMICOL && !status)
		{
			//cout << tok.GetLexeme() << "  in line "<< line<< endl;
			ParseError(line, "Syntactic error in Statement.");
			return false;
		}
		else if(tok != SEMICOL && tok != END && status)
		{
			ParseError(line, "Missing a semicolon.");
			return false;
		}
	}
	else
	{
		Parser::PushBackToken(tok);
	}	
	return status;
	
}

//Decl = Type IdentList 
//Type = INTEGER | REAL 
bool DeclStmt(istream& in, int& line) {
	bool status = false;
	LexItem tok;
	//cout << "in Decl" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	if(t == INT || t == FLOAT ) {
		status = IdentList(in, line, t);
		//cout<< "returning from IdentList" << " " << (status? 1: 0) << endl;
		if (!status)
		{
			ParseError(line, "Incorrect variable in Declaration Statement.");
			return status;
		}
		}
	else{
		Parser::PushBackToken(t);
		ParseError(line, "Incorrect Type.");
		return false;
	}
	return true;
}

bool Stmt(istream& in, int& line){
	bool status=true;
	//cout << "in Stmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {
	case INT: case FLOAT: 
		Parser::PushBackToken(t);
		status = DeclStmt(in, line);
		if(!status)
		{
			ParseError(line, "Incorrect Declaration Statement.");
			return status;
		}
		break;
	case IF: case WRITE: case IDENT: 
		Parser::PushBackToken(t);
		status = ControlStmt(in, line);
		if(!status)
		{
			ParseError(line, "Incorrect control Statement.");
			return status;
		}
		break;
	default:
		Parser::PushBackToken(t);
	}
	return status;
}
//Stmt is either a WriteStmt, RepeatStmt, IfStmt, AssigStmt, or CompStmt
//Stmt = AssigStmt | IfStmt | WriteStmt | RepeatStmt | CompStmt
bool ControlStmt(istream& in, int& line) {
	bool status;
	//cout << "in ContrlStmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case WRITE:
		status = WriteStmt(in, line);
		//cout << "After WriteStmet status: " << (status? true:false) <<endl;
		break;

	case IF:
		status = IfStmt(in, line);
		break;

	case IDENT:
		Parser::PushBackToken(t);
        status = AssignStmt(in, line);
		
		break;
		
		
	default:
		Parser::PushBackToken(t);
		return true;
	}

	return status;
}


//WriteStmt:= wi, ExpreList 
bool WriteStmt(istream& in, int& line) {
	LexItem t;
	//cout << "in WriteStmt" << endl;
		
	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression after Write");
		return false;
	}
	
	//Evaluate: print out the list of expressions values

	return ex;
}

//IfStmt:= if (Expr) ControlStmt} 
bool IfStmt(istream& in, int& line) {
	bool ex=false ; 
	LexItem t;
	Value newval, stmtval;
	//cout << "in IfStmt" << endl;
	if( (t=Parser::GetNextToken(in, line)) != LPAREN ) {
		
		ParseError(line, "Missing Left Parenthesis");
		return false;
	}
	
	ex = LogicExpr(in, line, newval);
	if( !ex ) {
		ParseError(line, "Missing if statement Logic Expression");
		return false;
	}

	if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
		
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}
	
	if(newval.IsBool())
	{
		if(newval.GetBool() )
		{
			//check syntax of stmt and interpret/execute the stmt
			bool st = ControlStmt(in, line);
			if( !st ) {
				ParseError(line, "Missing statement for if");
				return false;
			}
		}
		else
		{
			t = Parser::GetNextToken(in, line);
			while (t != SEMICOL)
			{
				t = Parser::GetNextToken(in, line);
			}
			Parser::PushBackToken(t);
		}
		
	}
	else{
		ParseError(line, "Run-Time Error-Illegal Type for If statement condition");
		return false;
	}
	
	return true;
}
//IdList:= IDENT {,IDENT}
bool IdentList(istream& in, int& line, LexItem type) {
	bool status = false;
	string identstr;
	//cout << "in IdList" << endl;
	LexItem tok = Parser::GetNextToken(in, line);
	if(tok == IDENT)
	{
		//set IDENT lexeme to the type tok value
		identstr = tok.GetLexeme();
		if (!(defVar.find(identstr)->second))
		{
			defVar[identstr] = true;
			SymTable[identstr] = type.GetToken();
		}	
		else
		{
			ParseError(line, "Variable Redefinition");
			return false;
		}
		
	}
	else
	{
		ParseError(line, "Missing Variable");
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok == COMMA) {
		status = IdentList(in, line, type);
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


//Var:= ident
bool Var(istream& in, int& line)
{
	//called only from the AssignStmt function
	string identstr;
	//cout << "in Var" << endl;
	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		
		if (!(defVar.find(identstr)->second))
		{
			ParseError(line, "Undeclared Variable");
			return false;
		}	
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
	//cout << "in AssignStmt" << endl;
	bool varstatus = false, status = false;
	LexItem t;
	
	varstatus = Var( in, line);
	//cout << "varstatus:" << varstatus << endl;
	
	if (varstatus){
		t = Parser::GetNextToken(in, line);
		//cout << t.GetLexeme() << endl;
		if (t == ASSOP){
			status = Expr(in, line, valQue);
			if(!status) {
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}
			
		}
		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			ParseError(line, "Missing Assignment Operator =");
			return false;
		}
	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
	return status;	
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;
	//cout << "in ExprList and before calling Expr" << endl;
	status = Expr(in, line);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}
	
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

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line) {
	//cout << "in Expr" << endl;
	bool t1 = Term(in, line);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == PLUS || tok == MINUS ) 
	{
		t1 = Term(in, line);
		if( !t1 ) 
		{
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
		//Evaluate: evaluate the expression for addition or subtraction
	}
	Parser::PushBackToken(tok);
	return true;
}

//Term:= SFactor {(*|/) SFactor}
bool Term(istream& in, int& line) {
	//cout << "in Term" << endl;
	bool t1 = SFactor(in, line);
	LexItem tok;
	//cout << "status of factor1: " << t1<< endl;
	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  || tok == REM)
	{
		t1 = SFactor(in, line);
		//cout << "status of factor2: " << t1<< endl;
		if( !t1 ) {
			ParseError(line, "Missing operand after operator");
			return false;
		}
		
		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		//Evaluate: evaluate the expression for multiplication or division
	}
	Parser::PushBackToken(tok);
	return true;
}

//SFactor = Sign Factor | Factor
bool SFactor(istream& in, int& line)
{
	LexItem t = Parser::GetNextToken(in, line);
	bool status;
	int sign = 0;
	if(t == MINUS )
	{
		sign = -1;
	}
	else if(t == PLUS)
	{
		sign = 1;
	}
	else
		Parser::PushBackToken(t);
		
	status = Factor(in, line, sign);
	return status;
}

//LogicExpr = Expr (== | <) Expr
bool LogicExpr(istream& in, int& line, Value & retVal)
{
	Value val1, val2;
	//cout << "in Logic Expr" << endl;
	bool t1 = Expr(in, line, val1);
	LexItem tok;
	
	if( !t1 ) {
		return false;
	}
	//cout << "in Logic Expr" << endl;
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	if ( tok == GTHAN  || tok == EQUAL) 
	{
		t1 = Expr(in, line, val2);
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after relational operator");
			return false;
		}
		if(tok == GTHAN)
		{
			retVal = (val1 > val2);
		}
		else if(tok == EQUAL)
		{
			retVal = (val2 == val2);
			//cout << "in Logic Expr 2" << endl;
		}
		if(retVal.GetType() == VERR)
		{
			ParseError(line, "Run-Time Error-Illegal Mixed Type operation");
			return false;
		}
		return true;
	}
	Parser::PushBackToken(tok);
	return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line, int sign) {
	//cout << "in Factor" << endl;
	LexItem tok = Parser::GetNextToken(in, line);
	

	if( tok == IDENT ) {
		//check if the var is defined 
		//int val;
		string lexeme = tok.GetLexeme();
		if (!(defVar.find(lexeme)->second))
		{
			ParseError(line, "Undefined Variable");
			return false;	
		}
		return true;
	}
	else if( tok == ICONST ) {
		
		return true;
	}
	else if( tok == SCONST ) {
		
		return true;
	}
	else if( tok == RCONST ) {
		
		return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line);
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}



