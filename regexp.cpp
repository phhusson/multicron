#include <pcreposix.h>
#include <string.h>
#include <string>

int regexp_match(const char *regexp, const char *val) {
	//If begin with '!' do an exact match
	if(regexp[0]=='!') {
		if(strcmp(regexp+1, val)==0)
			return 1;
		return 0;
	} 

	regex_t preg;
	int ret=regcomp(&preg, regexp, REG_UTF8);
	if(ret!=0) {
		char *errbuf=(char*)malloc(512);
		memset(errbuf, 0, 512);
		regerror(ret, &preg, errbuf, 512);
		throw std::string("Pattern won't compile :")+errbuf;
	}
	regmatch_t match;
	ret=regexec(&preg, val, 1, &match, 0);
	if(ret==0 && match.rm_so==0 && match.rm_eo==strlen(val))
		return 1;
	return 0;
}
