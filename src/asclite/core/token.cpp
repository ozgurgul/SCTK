/*
 * asclite
 * Author: Jerome Ajot, Nicolas Radde, Chris Laprun
 *
 * This software was developed at the National Institute of Standards and Technology by
 * employees of the Federal Government in the course of their official duties.  Pursuant to
 * Title 17 Section 105 of the United States Code this software is not subject to copyright
 * protection within the United States and is in the public domain. asclite is
 * an experimental system.  NIST assumes no responsibility whatsoever for its use by any party.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS."  With regard to this software, NIST MAKES NO EXPRESS
 * OR IMPLIED WARRANTY AS TO ANY MATTER WHATSOEVER, INCLUDING MERCHANTABILITY,
 * OR FITNESS FOR A PARTICULAR PURPOSE.
 */
 
/**
 * Internal representation of a token.
 * A token represent every informations needed on a word to align it.
 */

#include "token.h" // class's header file
#include "segment.h"

Logger* Token::logger = Logger::getLogger();

const char Token::FRAGMENT_MARKER = '-';
const char Token::BEGIN_OPTIONAL_MARKER = '(';
const char Token::END_OPTIONAL_MARKER = ')';

Token* Token::CreateWithDuration(int _startTime, int _duration, Segment* parent) 
{
	Token* token = new Token();
    token->segment = parent;
	return (Token*) token->InitWithDuration(_startTime, _duration);
}

Token* Token::CreateWithEndTime(int _startTime, int _endTime, Segment* parent) 
{
	Token* token = new Token();
	token->segment = parent;
	return (Token*) token->InitWithEndTime(_startTime, _endTime);
}

Token* Token::CreateWithDuration(int _startTime, int _duration, Segment* parent, string _text)
{
  Token* token = Token::CreateWithDuration(_startTime, _duration, parent);
  token->SetSourceText(_text);
  return token;
}

Token* Token::CreateWithEndTime(int _startTime, int _endTime, Segment* parent, string _text)
{
  Token* token = Token::CreateWithEndTime(_startTime, _endTime, parent);
  token->SetSourceText(_text);
  return token;
}

Token* Token::CreateWithEndTime(int _startTime, int _endTime, Segment* parent, string _text, Token* first_prec_tokens, ...)
{
    Token* token = Token::CreateWithEndTime(_startTime, _endTime, parent, _text);
    Token* tok = first_prec_tokens;
    va_list marker;
    va_start( marker, first_prec_tokens);     /* Initialize variable arguments. */
    
    while (tok != NULL)
    {
        tok->AddNextToken(token);
        token->AddPrecToken(tok);
        tok = va_arg(marker, Token*);
    }
    
    va_end(marker);
    return token;
}

// class constructor
Token::Token()
{
    optional = false;
    fragment = Token::NOT_FRAGMENT;
    hasConfidence = false;
	start = 0;
	size = 0;
	confidence = -1.0;
}

// class destructor
Token::~Token()
{
	prec.clear();
	next.clear();
}

void Token::UpdateCleanedUpTextIfNeeded(bool force) 
{
    start = 0;
    size = sourceText.size();
    //cout << "Source " << sourceText << endl;
    
    if(segment->UseOptionallyDeletable())
    {		
        optional = ( sourceText[start] == BEGIN_OPTIONAL_MARKER && sourceText[start + size - 1] == END_OPTIONAL_MARKER );
        
        if(optional) 
        {				
            ++start;
            size -= 2;
        }
    }
    else 
    {
        optional = false;
    }

    //cout << "Start " << start << endl;
    if(segment->AreFragmentsCorrect())
    {
        if (sourceText[start] == FRAGMENT_MARKER) 
        {
            fragment = Token::END_FRAGMENT; //end fragment
            ++start; 
            --size;
        }

        if (sourceText[start + size - 1] == FRAGMENT_MARKER)
        {
            fragment = Token::BEGIN_FRAGMENT; //begin fragment
            --size;
        }
    }
    else 
    {
        fragment = Token::NOT_FRAGMENT;
    }	
    //cout << " Start " << start << " size " << size << " str " << sourceText.substr(start, size) << " frag? " << fragment <<  endl;
}

void Token::LinkTokens(Token *next)
{
	next->AddPrecToken(this);
	AddNextToken(next);
}

void Token::UnlinkNextToken(Token *next)
{
	vector<Token *>::iterator tokNext = this->next.begin();
	while (tokNext != this->next.end() && (*tokNext) != next)
		tokNext++;

	if (tokNext == this->next.end()){
		char buffer[BUFFER_SIZE];
		sprintf(buffer, "Can't unlink next token %s from %s",next->ToString().c_str(), this->ToString().c_str());
		LOG_FATAL(logger, buffer);
		exit(1);
	}
	this->next.erase(tokNext);
}

void Token::UnlinkTokens(Token *next)
{
	vector<Token *>::iterator tokNext = this->next.begin();
	while (tokNext != this->next.end() && (*tokNext) != next)
		tokNext++;

	vector<Token *>::iterator nextPrec = next->prec.begin();
	while (nextPrec != next->prec.end() && (*nextPrec) != this)
		nextPrec++;

	if (tokNext == this->next.end() || nextPrec == next->prec.end()){
		char buffer[BUFFER_SIZE];
		sprintf(buffer, "Can't unlink tokens %s and %s",this->ToString().c_str(), next->ToString().c_str());
		LOG_FATAL(logger, buffer);
		exit(1);
	}

	this->next.erase(tokNext);
	next->prec.erase(nextPrec);
}

string Token::GetText()
{
	UpdateCleanedUpTextIfNeeded(false);
	return sourceText.substr(start, size);
}

void Token::SetSourceText(string text)
{
	sourceText = text;
	UpdateCleanedUpTextIfNeeded(true);
}

string Token::GetTextInLowerCase()
{
	string tmp = GetText();
	transform(tmp.begin(), tmp.end(), tmp.begin(), (int(*)(int))tolower);
	return tmp;
}

// sets the value of confidence
void Token::SetConfidence(float x)
{
	confidence = x;
	hasConfidence = true;
}

/**
 * Return if the Token is Optionnaly Deletable/Insertable.
 */
bool Token::IsOptional()
{
	UpdateCleanedUpTextIfNeeded(false);
	return optional;
}

/**
 * Return if the token is a Fragment
 */
int Token::GetFragmentStatus()
{
	UpdateCleanedUpTextIfNeeded(false);
    return fragment;
}

/*
 * Return true if the two tokens are equivalent in a 
 * Speech recognition way.
 */
bool Token::IsEquivalentTo(Token* token)
{
    string tok1;
    string tok2;
    
    if (segment->PerformCaseSensitiveAlignment()) //case sensitive
    {
        tok1 = GetText();
        tok2 = token->GetText();
    }
    else
    {
        tok1 = GetTextInLowerCase();
        tok2 = token->GetTextInLowerCase();
    }
	
	if (segment->AreFragmentsCorrect()) 
    {
		int otherFrag = token->GetFragmentStatus();
		
		if(fragment == NOT_FRAGMENT) 
        {
			if(otherFrag == NOT_FRAGMENT) 
            {
				return tok1 == tok2;
			}
            else if (otherFrag == BEGIN_FRAGMENT) 
            {
                return (tok1.find(tok2, 0) == 0);
			} 
            else 
            {
				return (tok1.find(tok2, tok1.size()-tok2.size()) != string::npos);					
			}
		}
        else if (fragment == BEGIN_FRAGMENT)
        {			
			if(otherFrag == NOT_FRAGMENT) 
            {
				return (tok2.find(tok1, 0) == 0);
			} 
            else if (otherFrag == BEGIN_FRAGMENT) 
            {
				 return ((tok1.find(tok2, 0) != string::npos) || (tok2.find(tok1, 0) != string::npos));
			}
            else 
            {
				return false;
			}
			
		} 
        else 
        { // fragment == END_FRAGMENT
			if(otherFrag == NOT_FRAGMENT)
            {
				return (tok2.find(tok1, tok2.size()-tok1.size()) != string::npos);
			} 
            else if (otherFrag == BEGIN_FRAGMENT) 
            {
				return false;
			} 
            else 
            {
				uint s1 = tok1.size();
				uint s2 = tok2.size();
				return (tok1.find(tok2, s1 - s2) != string::npos) || (tok2.find(tok1, s2 - s1) != string::npos);
			}
		}
	} 
    else 
    {
        return tok1 == tok2;
    }
}

bool Token::Equals(Token* token) 
{
	if(!TimedObject::Equals(token))
		return false;
	
	return GetSourceText() == token->GetSourceText()/* && prec == token->prec && next == token->next*/;
}

string Token::ToString() 
{
    string temp = "";
    
    for (uint i = 0 ; i < prec.size() ; i++)
    {
        if (i==0) 
            temp = temp + "[" + prec[i]->GetText();
        else  
            temp = temp + "," + prec[i]->GetText();
    }

    if (prec.size() != 0) 
        temp += "] ";
    
    temp += GetText();
	
    for (uint i = 0 ; i < next.size() ; i++)
    {
        if (i==0) 
            temp = temp + " [" + next[i]->GetText();
        else  
            temp = temp + "," + next[i]->GetText();
    }
  
    if (next.size() != 0) 
        temp += "]";
		
    return temp;
}

string Token::GetCSVInformation()
{
	string conf = "";
	
	if(hasConfidence)
	{
		char buffer [BUFFER_SIZE];
		sprintf(buffer, "%.3f", confidence);
		conf = string(buffer);
	}
	
	string tknbt = "";
	
	if(GetStartTime() >= 0)
	{
		char buffer [BUFFER_SIZE];
		sprintf(buffer, "%.3f", ((double)GetStartTime())/1000.0);
		tknbt = string(buffer);
	}
	
	string tknet = "";
	
	if(GetEndTime() >= 0)
	{
		char buffer [BUFFER_SIZE];
		sprintf(buffer, "%.3f", ((double)GetEndTime())/1000.0);
		tknet = string(buffer);
	}
	
	string segbt = "";
	
	if(segment->GetStartTime() >= 0)
	{
		char buffer [BUFFER_SIZE];
		sprintf(buffer, "%.3f", ((double)segment->GetStartTime())/1000.0);
		segbt = string(buffer);
	}
	
	string seget = "";
	
	if(segment->GetEndTime() >= 0)
	{
		char buffer [BUFFER_SIZE];
		sprintf(buffer, "%.3f", ((double)segment->GetEndTime())/1000.0);
		seget = string(buffer);
	}
	
	string spkr = segment->GetSpeakerId();
	string tkntext = sourceText;
	
	char buffer1[BUFFER_SIZE];
	sprintf(buffer1, "%li", GetsID());
	string tknsid = string(buffer1);
	
	char buffer2[BUFFER_SIZE];
	sprintf(buffer2, "%li", segment->GetsID());
	string segsid = string(buffer2);
	
	string listprevtkn = string("");
	
	for(size_t i=0; i< prec.size(); ++i)
	{
		char buffer3[BUFFER_SIZE];
		sprintf(buffer3, "%li", prec[i]->GetsID());
		string tknid = string(buffer3);
		
		listprevtkn += tknid;
		
		if(i < prec.size() -1 )
			listprevtkn += "|";
	}
	
	string listnexttkn = string("");
	
	for(size_t i=0; i< next.size(); ++i)
	{
		char buffer3[BUFFER_SIZE];
		sprintf(buffer3, "%li", next[i]->GetsID());
		string tknid = string(buffer3);
		
		listnexttkn += tknid;
		
		if(i < next.size() -1 )
			listnexttkn += "|";
	}
	
	return(segsid + "," + segbt + "," + seget + "," + spkr + "," + tknsid + "," + tknbt + "," + tknet + "," + tkntext + "," + conf + "," + listprevtkn + "," + listnexttkn);
}