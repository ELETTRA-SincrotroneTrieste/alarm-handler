/*
 * alarm_grammar.h
 *
 * $Author:  $
 *
 * $Revision:  $
 *
 * $Log: alarm_grammar.h,v $
 *
 *
 * copyleft: Sincrotrone Trieste S.C.p.A. di interesse nazionale
 *           Strada Statale 14 - km 163,5 in AREA Science Park
 *           34012 Basovizza, Trieste ITALY
 */


#ifndef ALARM_GRAMMAR_H_
#define ALARM_GRAMMAR_H_

//--------------------------NOTE!!---------------------------------
//----if a grammar is intended to be used in multithreaded code----
//----it is required to "#define BOOST_SPIRIT_THREADSAFE" and------
//----to link against Boost.Threads--------------------------------
//-----------------------------------------------------------------
#if BOOST_VERSION  < 103600 
#ifndef BOOST_SPIRIT_THREADSAFE 
#define BOOST_SPIRIT_THREADSAFE 
#endif

#ifndef PHOENIX_THREADSAFE
#define PHOENIX_THREADSAFE
#endif
#endif

#if BOOST_VERSION  < 103600    
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/assign_actor.hpp>			//for assign_a
#include <boost/spirit/actor/push_back_actor.hpp>		//for push_back_a
#include <boost/spirit/actor/insert_at_actor.hpp>		//for insert_at_a
#include <boost/spirit/actor/clear_actor.hpp>			//for clear_a
#include <boost/spirit/symbols/symbols.hpp>				//for symbol table
#include <boost/spirit/utility/confix.hpp>				//for confix
#include <boost/spirit/phoenix/primitives.hpp>			//needed for "var" in group rule
#include <boost/spirit/phoenix/operators.hpp>			//needed for "var" in group rule
#include <boost/spirit/phoenix/functions.hpp>
#else
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>			//for assign_a
#include <boost/spirit/include/classic_push_back_actor.hpp>		//for push_back_a
#include <boost/spirit/include/classic_insert_at_actor.hpp>		//for insert_at_a
#include <boost/spirit/include/classic_clear_actor.hpp>			//for clear_a
#include <boost/spirit/include/classic_symbols.hpp>				//for symbol table
#include <boost/spirit/include/classic_confix.hpp>				//for confix
#include <boost/spirit/include/phoenix1_primitives.hpp>			//needed for "var" in group rule
#include <boost/spirit/include/phoenix1_operators.hpp>			//needed for "var" in group rule
#include <boost/spirit/include/phoenix1_functions.hpp>
#endif

//#include <boost/spirit/attribute.hpp>					//for closure

//#include <boost/spirit/tree/ast.hpp>					//for ast parse trees (in tree_formula)

#include <iostream>
#include <string>
#include <map>

#include "event_table.h"
#include "formula_grammar.h"

#define NAME_KEY			"name"
#define FORMULA_KEY			"formula"
#define DELAY_KEY			"time_threshold"
#define LEVEL_KEY			"level"
#define SILENT_TIME_KEY		"silent_time"
#define GROUP_KEY			"group"
#define MESSAGE_KEY			"message"
#define ON_COMMAND_KEY		"on_command"
#define OFF_COMMAND_KEY		"off_command"
#define KEY(S_VAL)  		S_VAL "="

////////////////////////////////////////////////////////////////////////////
using namespace std; 
#if BOOST_VERSION  < 103600 
using namespace boost::spirit;
#else
using namespace boost::spirit::classic;
#endif
using namespace phoenix;								//needed for "var" in group rule

////////////////////////////////////////////////////////////////////////////
//
//  Semantic actions for alarm gramamr
//
////////////////////////////////////////////////////////////////////////////
#if 0
struct push_back_impl
{
    template <typename Container, typename Item>
    struct result
    {
        typedef void type;
    };

    template <typename Container, typename Item>
    void operator()(Container& c, Item const& item) const
    {
        c.push_back(item);
    }
};

function<push_back_impl> const push_back = push_back_impl();

struct insert_map_impl
{
    template <typename Container, typename Item1, typename Item2>
    struct result
    {
        typedef void type;
    };

    template <typename Container, typename Item1, typename Item2>
    void operator()(Container& c, Item1 const& item1, Item2 const& item2) const
    {
        c.insert(make_pair(item1, item2));
    }
};

function<insert_map_impl> const insert_map = insert_map_impl();
#endif
////////////////////////////////////////////////////////////////////////////
//
//  alarm grammar
//
////////////////////////////////////////////////////////////////////////////
struct alarm_parse : public grammar<alarm_parse>
{
	alarm_t &m_alarm;
	
	symbols<unsigned int> sym_grp;
	
	alarm_parse(alarm_t &a) \
		: m_alarm(a) 
		{
			//init symbol table with group defined in alarm_t::grp_str
			map<string,unsigned int>::iterator i = m_alarm.grp_str.begin();
			while(i != m_alarm.grp_str.end())
			{
				sym_grp.add(i->first.c_str(), i->second);
				i++;
			}		
		}   
   
    template <typename ScannerT>
    struct definition
    {
        definition(alarm_parse const& self)
        {
		//-------BOOST.SPIRIT GLOSSARY-------  
		// >>			: sequence
		// |			: union (i.e. alternative)
		// -			: difference
		// *			: kleene star (matches 0 or more times)
		// +			: positive (matches 1 or more times)
		// !			: optional (matches 0 or 1 time)
		// str_p		: matches string
		// ch_p			: matches char
		// hex_p		: 
		// alnum_p		: matches alpha-numeric characters
		// anychar_p	: matches any single character (including the null terminator: '\0')
		// lexemd_d		: turns off white space skipping
		// confix		: recognize a sequence of: an opening, an expression and a closing
		// assign_a		:
		// push_back_a	:
    	
		//std::pair<string, vector<string> > temp;    	
    	
            expression
            	=	discard_node_d
            		[
            		name_alm
            			[
            				assign_a(self.m_alarm.name)			//save name in alarm_t
						]
					]		//discard_node_d					
            		>> 
            			root_node_d
            			[ 
            			formula	
            				[
            					assign_a(self.m_alarm.formula)	//save formula in alarm_t
            				]
            			]		//root_node_d
					>> *(discard_node_d[option])
			;

            option
				=	discard_node_d[time_threshold] |
					discard_node_d[level] |
					discard_node_d[silent_time] |
					discard_node_d[group] |
					discard_node_d[msg] |
					discard_node_d[on_command] |
					discard_node_d[off_command]
			;

            //------------------------------ALARM NAME--------------------------------------            
            symbol
            	=	alnum_p | '.' | '_'  | '-' | '+'				//any alpha numeric char plus '.', '_', '-', '+'
            	;	
            symbol_attr_name
            	=	alnum_p | '_'									//any alpha numeric char plus '_'
            	;
            name
            	=	(+symbol) >> '/' >> (+symbol) 
            		>> '/' >> (+symbol) >> '/' >> (+symbol_attr_name)
            	;
            name_alm
            	=	(+symbol_attr_name)
            	; 
			//------------------------------LEVEL--------------------------------------	
			level
				=	discard_node_d[str_p(KEY(LEVEL_KEY))] >>
					//lexeme_d[(+alnum_p)]		//match only possible levels?? (fault, log, ...)
					//(+(alnum_p-'\t'))
					(str_p("fault") | str_p("warning") | str_p("log"))
					[
						assign_a(self.m_alarm.lev)		//save level in alarm_t
					]
				;
			//------------------------------GROUP--------------------------------------	
			group
				=	discard_node_d[str_p(KEY(GROUP_KEY))] >>
					self.sym_grp				//match only group defined in sym_grp symbol table
					[
						var(self.m_alarm.grp) |= arg1			//using phoenix::var
					]
					>> *(
						ch_p('|') 
						>> self.sym_grp			//match only group defined in sym_grp symbol table
							[
								var(self.m_alarm.grp) |= arg1	//using phoenix::var
							]
						)
				;
			//------------------------------MESSAGE--------------------------------------	
			msg
				=	discard_node_d[str_p(KEY(MESSAGE_KEY))] >>
					ch_p('"')
					>> (+(anychar_p - '\"')) 		//one ore more char except '"'
							[
								assign_a(self.m_alarm.msg)
							]					
					>> '"'
				;
			//---------------------------TIME THRESHOLD----------------------------------	
			time_threshold 
				=	discard_node_d[str_p(KEY(DELAY_KEY))] >>
					(uint_p
					[
						assign_a(self.m_alarm.time_threshold)
					]
					| epsilon_p)
				;
			//-----------------------------SILENT TIME------------------------------------
			silent_time
				= discard_node_d[str_p(KEY(SILENT_TIME_KEY))] >>
					(int_p
					[
						assign_a(self.m_alarm.silent_time)
					]
					| epsilon_p)
				;
			//-----------------------------ON COMMAND------------------------------------
			on_command
				= discard_node_d[str_p(KEY(ON_COMMAND_KEY))] >>
						(discard_node_d
						[
							name
							[
								assign_a(self.m_alarm.cmd_name_a)		//save cmd_name_a in alarm_t
							]
						]		//discard_node_d
						| epsilon_p)
				;
			//-----------------------------OFF COMMAND------------------------------------
			off_command
				= discard_node_d[str_p(KEY(OFF_COMMAND_KEY))] >>
						(discard_node_d
						[
							name
							[
								assign_a(self.m_alarm.cmd_name_n)		//save cmd_name_a in alarm_t
							]
						]		//discard_node_d
						| epsilon_p)
				;
        }
        
		typedef rule<ScannerT> rule_t;	
		rule_t expression, event, option;
        rule<typename lexeme_scanner<ScannerT>::type> symbol;					//needed to use lexeme_d in rule name
        rule<typename lexeme_scanner<ScannerT>::type> symbol_attr_name;		//needed to use lexeme_d in rule name
        rule_t name, name_alm, val, token, oper, msg, group, level, time_threshold, silent_time, on_command, off_command;
		formula_grammar formula;
		
		rule_t const&					
        start() const { return expression; }
    };
};


#endif /*ALARM_GRAMMAR_H_*/
