/*
 * formula_grammar.h
 *
 * $Author: graziano $
 *
 * $Revision: 1.5 $ 
 *
 * $Log: formula_grammar.h,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */
#ifndef FORMULA_GRAMMAR_H_
#define FORMULA_GRAMMAR_H_

#include <boost/version.hpp>

//#define BOOST_SPIRIT_NO_TREE_NODE_COLLAPSING  //test trying to have node also if formula of type (ev/ev/ev/ev) 27/02/2008

#if BOOST_VERSION  < 103600  
#ifndef BOOST_SPIRIT_THREADSAFE 
#define BOOST_SPIRIT_THREADSAFE 
#endif
#endif

#if BOOST_VERSION  < 103600
#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>
//for ast parse trees (in tree_formula)
#include <boost/spirit/symbols/symbols.hpp>				//for symbol table
using namespace boost::spirit;
#else
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_ast.hpp>
//for ast parse trees (in tree_formula)
#include <boost/spirit/include/classic_symbols.hpp>				//for symbol table
using namespace boost::spirit::classic;
#endif


using namespace std;  



/*typedef char const*                    iterator_t;       
typedef node_val_data_factory<unsigned int>    factory_t;
typedef tree_match<iterator_t, factory_t>        parse_tree_match_t;   
typedef tree_parse_info<iterator_t, factory_t>    tree_parse_t;   
typedef parse_tree_match_t::tree_iterator        iter_t;*/
typedef node_val_data<iterator_t, unsigned int>    node_t;
typedef tree_node<node_t>                tree_node_t;

/*static unsigned int stat_tmp;
static void Save_Stat (unsigned int val)
{
	stat_tmp = val;
}

static void Assign_Stat (tree_node_t & node, const iterator_t & begin, const iterator_t & end)
{
	node.value.value(stat_tmp);
}*/

static unsigned int stat_tmp;
struct Save_Stat
{   
	void operator () (unsigned int val) const
	{
		stat_tmp = val;
	}
};

struct Assign_Stat 
{  
	void operator () (tree_node_t & node, const iterator_t & begin, const iterator_t & end) const
	{
		node.value.value(stat_tmp);
	}
};

//TODO: duplicated from alarm.h
enum _AlarmStateEnum {
	_NORM,
	_UNACK,
	_ACKED,
	_RTNUN,
	_SHLVD,
	_DSUPR,
	_OOSRV,
	_ERROR
} ;

static vector<string> quality_labels;

struct formula_grammar : public grammar<formula_grammar>
{

	/*formula_t &m_formula;

	symbols<unsigned int> sym_grp;

	formula_grammar(formula_t &f) \
		: m_formula(f)
		{

		}*/

    static const int val_rID = 1;
    static const int val_hID = 2; 
    static const int val_stID = 3;    
    static const int event_ID = 4;
    static const int nameID = 5;
    static const int indexID = 6; 
    static const int funcID = 7;
    static const int logical_exprID = 8;
    static const int bitwise_exprID = 9;
    static const int equality_exprID = 10;
    static const int compare_exprID = 11;
    static const int add_exprID = 12;
    static const int mult_exprID = 13;
    static const int expr_atomID = 14;
    static const int shift_exprID = 15;
    static const int unary_exprID = 16;
    static const int val_stringID = 17;	//TODO: OK ?
    static const int func_dualID = 18;
    static const int logical_expr_parenID = 19;
    static const int cond_exprID = 20;
    static const int exprID = 21;
    static const int nonempty_exprID = 22;
    static const int val_qualityID = 23;
    static const int val_alarm_enum_stID = 24;
    static const int propertyID = 25;

    
    symbols<unsigned int> tango_states;
    symbols<unsigned int> attr_quality;
    
    symbols<unsigned int> alarm_enum_states;

	formula_grammar() 
	{
		tango_states.add("ON", (unsigned int)Tango::ON);
		tango_states.add("OFF", (unsigned int)Tango::OFF);
		tango_states.add("CLOSE", (unsigned int)Tango::CLOSE);
		tango_states.add("OPEN", (unsigned int)Tango::OPEN);
		tango_states.add("INSERT", (unsigned int)Tango::INSERT);
		tango_states.add("EXTRACT", (unsigned int)Tango::EXTRACT);
		tango_states.add("MOVING", (unsigned int)Tango::MOVING);
		tango_states.add("STANDBY", (unsigned int)Tango::STANDBY);
		tango_states.add("FAULT", (unsigned int)Tango::FAULT);
		tango_states.add("INIT", (unsigned int)Tango::INIT);
		tango_states.add("RUNNING", (unsigned int)Tango::RUNNING);
		tango_states.add("ALARM", (unsigned int)Tango::ALARM);
		tango_states.add("DISABLE", (unsigned int)Tango::DISABLE);
		tango_states.add("UNKNOWN", (unsigned int)Tango::UNKNOWN);

		quality_labels.push_back("ATTR_VALID");
		attr_quality.add(quality_labels.back().c_str(), (unsigned int)Tango::ATTR_VALID);
		quality_labels.push_back("ATTR_INVALID");
		attr_quality.add(quality_labels.back().c_str(), (unsigned int)Tango::ATTR_INVALID);
		quality_labels.push_back("ATTR_ALARM");
		attr_quality.add(quality_labels.back().c_str(), (unsigned int)Tango::ATTR_ALARM);
		quality_labels.push_back("ATTR_CHANGING");
		attr_quality.add(quality_labels.back().c_str(), (unsigned int)Tango::ATTR_CHANGING);
		quality_labels.push_back("ATTR_WARNING");
		attr_quality.add(quality_labels.back().c_str(), (unsigned int)Tango::ATTR_WARNING);


		alarm_enum_states.add("NORM", (unsigned int)_NORM);
		alarm_enum_states.add("UNACK", (unsigned int)_UNACK);
		alarm_enum_states.add("ACKED", (unsigned int)_ACKED);
		alarm_enum_states.add("RTNUN", (unsigned int)_RTNUN);
		alarm_enum_states.add("SHLVD", (unsigned int)_SHLVD);
		alarm_enum_states.add("DSUPR", (unsigned int)_DSUPR);
		alarm_enum_states.add("OOSRV", (unsigned int)_OOSRV);
		alarm_enum_states.add("ERROR", (unsigned int)_ERROR);
	}   
   
    template <typename ScannerT>
    struct definition
    {
        
/*        typedef
            scanner<
                typename ScannerT::iterator_t,
                scanner_policies<
                    typename ScannerT::iteration_policy_t,
                    ast_match_policy<
                        typename ScannerT::match_policy_t::iterator_t,
                        typename ScannerT::match_policy_t::factory_t
                    >,
                    typename ScannerT::action_policy_t
                >
            > ast_scanner;*/  

		//typedef scanner_list< ScannerT, phrase_scanner_t > scanners; //try using multiple scanners

        definition(formula_grammar const& self)
        {      
            symbol
            	=	(alnum_p | '.' | '_' | '-' | '+')				//any alpha numeric char plus '.', '_', '-'
            	;
            symbol_attr
            	=	(alnum_p | '_' )								//any alpha numeric char plus '_' for attribute names
            	;
            //------------------------------ALARM NAME--------------------------------------	
            name
#if BOOST_VERSION  < 103600              
            	=	token_node_d[
#else            	
            	=	reduced_node_d[
#endif
            			lexeme_d[							//needed to ignore "space_p" (skip white spaces) evaluating name
            				!("tango://" >> (+symbol) >> ':' >> uint_p >> "/")	//eventually match FQDN
            				>> (+symbol) >> '/' >> (+symbol)
            				>> '/' >> (+symbol) >> '/' >> (+symbol_attr)
            			]
            		]
//            	=	repeat_p(3)[(+symbol) >> ch_p('/')] >> (+symbol) 
            	; 
            index
            	= 	inner_node_d[ch_p('[') >> uint_p >> ch_p(']')]
            	;
            property
            	= 	str_p(".quality")
					| str_p(".alarm")
					| str_p(".normal")
            	;
            //------------------------------FORMULA--------------------------------------	   
			val_r
#if BOOST_VERSION  < 103600              
				=	leaf_node_d[real_p]
#else            	
            	=	reduced_node_d[real_p]
#endif				
				;
			val_h
#if BOOST_VERSION  < 103600  		
				=	token_node_d[(str_p("0x") >> hex_p)]		//token_node_d here to create only one node with all chars
#else            	
            	=	reduced_node_d[(str_p("0x") >> hex_p)]		//token_node_d here to create only one node with all chars
#endif					
				;
			val_st
				=
					//access_node_d[self.tango_states[&Save_Stat]][&Assign_Stat]	//save Tango::state value in node
					access_node_d[self.tango_states[Save_Stat()]][Assign_Stat()]	//save Tango::state value in node
            	;
			val_quality
				=
					//access_node_d[self.attr_quality[&Save_Stat]][&Assign_Stat]	//save Tango::state value in node
					access_node_d[self.attr_quality[Save_Stat()]][Assign_Stat()]	//save Tango::state value in node
            	;
			val_alarm_enum_st
				=
					//access_node_d[self.alarm_enum_states[&Save_Stat]][&Assign_Stat]	//save Tango::state value in node
					access_node_d[self.alarm_enum_states[Save_Stat()]][Assign_Stat()]	//save Tango::state value in node
            	;
            val_string
#if BOOST_VERSION  < 103600
            	=	token_node_d[
#else
            	=	reduced_node_d[
#endif
            	 	    lexeme_d[							//to conserve white spaces
            				ch_p('\'')
            				>> (+(anychar_p - '\'')) 		//one ore more char except '"'
            				>> '\''
            			]
            		]
//            	=	repeat_p(3)[(+symbol) >> ch_p('/')] >> (+symbol)
            	;

			event_
				=	name
					>> !( (index)
						| (property)
						)
				;				

			/*top = ternary_if;

			ternary_if
			   =	logical_expr_paren
			   	   >> !(root_node_d[str_p("?")] >> logical_expr_paren >> discard_node_d[ch_p(':')] >> logical_expr_paren)
			    ;*/

			cond_expr = logical_expr >> *(root_node_d[ch_p('?')] >> cond_expr >> discard_node_d[ch_p(':')] >> cond_expr);

			/*top = logical_expr
				;*/

            logical_expr
                = 	bitwise_expr
                 	>> *(	(root_node_d[str_p("&&")] >> bitwise_expr)
                 		|	(root_node_d[str_p("||")] >> bitwise_expr)
                 		)
                 ;

            bitwise_expr
                = 	equality_expr
                	 >> *(	(root_node_d[ch_p('&')] >> equality_expr)
                	 	| 	(root_node_d[ch_p('|')] >> equality_expr) 
                	 	| 	(root_node_d[ch_p('^')] >> equality_expr)
                	 	)
                ;
                
            equality_expr
                = 	compare_expr
                	>> *(	(root_node_d[str_p("==")] >> compare_expr)
                		| 	(root_node_d[str_p("!=")] >> compare_expr)
                		)
				;

            compare_expr
                = 	shift_expr
                 	>> *(	(root_node_d[str_p("<=")] >> add_expr) 
                 		| 	(root_node_d[str_p(">=")] >> add_expr) 
                 		|	(root_node_d[ch_p('<')] >> add_expr) 
                 		| 	(root_node_d[ch_p('>')] >> add_expr) 
                 		)
                 ;
			shift_expr
                = 	add_expr
                	>> *(	(root_node_d[str_p("<<")] >> add_expr)
                		| 	(root_node_d[str_p(">>")] >> add_expr)
                		)
                ;
            add_expr
                = 	mult_expr
                	>> *(	(root_node_d[ch_p('+')] >> mult_expr)
                		|	(root_node_d[ch_p('-')] >> mult_expr)
                		)
                ;
            mult_expr
                = 	unary_expr
                	>> *(	(root_node_d[ch_p('*')] >> unary_expr)
                		| 	(root_node_d[ch_p('/')] >> unary_expr)
                		)
                ;

            unary_expr
            	=	(	expr_atom
            		 |	function
            		 |	function_dual
            		 |	(root_node_d[ch_p('+')] >> expr_atom)
            		 |	(root_node_d[ch_p('-')] >> expr_atom)
            		 |	(root_node_d[ch_p('!')] >> expr_atom)
            		// |	(root_node_d[ch_p('~')] >> expr_atom)	//TODO
            		)
            	;

            function
            	=	( root_node_d[str_p("abs")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		| root_node_d[str_p("cos")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		| root_node_d[str_p("sin")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		| root_node_d[str_p("quality")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		| root_node_d[str_p("AND")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		| root_node_d[str_p("OR")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(')')])	//TODO: ? not expr_atom ?
            		)
            	;
            function_dual
        	=	(	(root_node_d[str_p("max")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(',')] >> cond_expr >> discard_node_d[ch_p(')')]))
        		//|	(root_node_d[str_p("max")] >> (inner_node_d[ch_p('(') >> discard_node_d[ch_p('(')] >> logical_expr >> discard_node_d[ch_p(')')] >> discard_node_d[ch_p(',')] >> discard_node_d[ch_p('(')] >> logical_expr >> discard_node_d[ch_p(')')] >> ')']))
        		|	(root_node_d[str_p("min")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(',')] >> cond_expr >> discard_node_d[ch_p(')')]))
        		//|	(root_node_d[str_p("min")] >> (inner_node_d[ch_p('(') >> discard_node_d[ch_p('(')] >> logical_expr >> discard_node_d[ch_p(')')] >> discard_node_d[ch_p(',')] >> discard_node_d[ch_p('(')] >> logical_expr >> discard_node_d[ch_p(')')] >> ')']))
				|	(root_node_d[str_p("pow")] >> (discard_node_d[ch_p('(')] >> cond_expr >> discard_node_d[ch_p(',')] >> cond_expr >> discard_node_d[ch_p(')')]))
        		)
            	//=	*(	(root_node_d[str_p("max")] >> (inner_node_d[ch_p('(') >> logical_expr_paren >> discard_node_d[ch_p(',')] >> logical_expr_paren >> ')']))
            	//	|	(root_node_d[str_p("min")] >> (inner_node_d[ch_p('(') >> logical_expr_paren >> discard_node_d[ch_p(',')] >> logical_expr_paren >> ')']))
            		//|	(root_node_d[str_p("min")] >> (discard_node_d[ch_p('(')] >> logical_expr_paren >> discard_node_d[ch_p(',')] >> logical_expr_paren >> discard_node_d[ch_p(',')]))
            		//|	(root_node_d[str_p("min")] >> (ch_p('(') >> expr_atom >> ch_p(',') >> expr_atom >> ch_p(',')))
            		//)
            	;

            non_empty_expression = cond_expr;
            top = non_empty_expression | epsilon_p;
           // top = non_empty_expression;

            expr_atom
                =	//val_h | val_r
					event_
					| val_h | val_r | val_st  | val_quality | val_alarm_enum_st | val_string
                	//| (inner_node_d[ch_p('(') >> non_empty_expression >> ')'])
               		| (discard_node_d[ch_p('(')] >> non_empty_expression >> discard_node_d[ch_p(')')])
                ;
            /*logical_expr_paren
            	=	(discard_node_d[ch_p('(')] >> logical_expr >> discard_node_d[ch_p(')')])
                	| logical_expr
                ;*/
        }
        
        rule<ScannerT> top;
        //rule<ScannerT> symbol;
        rule<typename lexeme_scanner<ScannerT>::type> symbol;					//needed to use lexeme_d in rule name
        rule<typename lexeme_scanner<ScannerT>::type> symbol_attr;					//needed to use lexeme_d in rule name
        rule<ScannerT, parser_context<>, parser_tag<val_rID> > val_r;
        rule<ScannerT, parser_context<>, parser_tag<val_hID> > val_h;
        rule<ScannerT, parser_context<>, parser_tag<val_stID> > val_st;        
        rule<ScannerT, parser_context<>, parser_tag<event_ID> > event_;
        rule<ScannerT, parser_context<>, parser_tag<logical_exprID> > logical_expr;
        rule<ScannerT, parser_context<>, parser_tag<bitwise_exprID> > bitwise_expr;
        rule<ScannerT, parser_context<>, parser_tag<shift_exprID> > shift_expr;
        rule<ScannerT, parser_context<>, parser_tag<equality_exprID> > equality_expr;
        rule<ScannerT, parser_context<>, parser_tag<compare_exprID> > compare_expr;
        rule<ScannerT, parser_context<>, parser_tag<add_exprID> > add_expr;
        rule<ScannerT, parser_context<>, parser_tag<mult_exprID> > mult_expr;
		rule<ScannerT, parser_context<>, parser_tag<unary_exprID> > unary_expr;        
        rule<ScannerT, parser_context<>, parser_tag<expr_atomID> > expr_atom;
        rule<ScannerT, parser_context<>, parser_tag<funcID> > function;
		rule<ScannerT, parser_context<>, parser_tag<nameID> > name;
		rule<ScannerT, parser_context<>, parser_tag<indexID> > index;
		rule<ScannerT, parser_context<>, parser_tag<val_stringID> > val_string;
		rule<ScannerT, parser_context<>, parser_tag<func_dualID> > function_dual;
		rule<ScannerT, parser_context<>, parser_tag<logical_expr_parenID> > logical_expr_paren;
		rule<ScannerT, parser_context<>, parser_tag<cond_exprID> > cond_expr;
		rule<ScannerT, parser_context<>, parser_tag<nonempty_exprID> > non_empty_expression;
		rule<ScannerT, parser_context<>, parser_tag<exprID> > expression;
		rule<ScannerT, parser_context<>, parser_tag<val_qualityID> > val_quality;
		rule<ScannerT, parser_context<>, parser_tag<val_alarm_enum_stID> > val_alarm_enum_st;
		rule<ScannerT, parser_context<>, parser_tag<propertyID> > property;


        rule<ScannerT> const&
        start() const { return top; }      
        
    };
};

#endif /*FORMULA_GRAMMAR_H_*/
