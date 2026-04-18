#include "./simple_regex.h"

NODISCARD MAYBE_UNUSED static inline SimpleRegexResult
new_simple_regex_result_error(tstr const error) {
	return (SimpleRegexResult){ .is_error = true, .data = { .error = error } };
}

NODISCARD MAYBE_UNUSED static inline SimpleRegexResult
new_simple_regex_result_ok(SimpleRegex const ok) {
	return (SimpleRegexResult){ .is_error = false, .data = { .ok = ok } };
}

#define ERR_BUF_SIZE 512

NODISCARD SimpleRegexResult simple_regex_compile(const tstr* const str) {
	// see: https://json-schema.org/understanding-json-schema/reference/regular_expressions
	// and man regcomp

	SimpleRegex result = ZERO_STRUCT(SimpleRegex);

	// flags for the json schema regex and don't use capture groups, only return overall match
	int compile_flags = REG_NEWLINE | REG_NOSUB;

	int compile_result = regcomp(&result.regex, tstr_cstr(str), compile_flags);

	if(compile_result != 0) {

		char* errbuf = malloc(sizeof(char) * (ERR_BUF_SIZE + 1));

		size_t bytes_written = regerror(compile_result, NULL, errbuf, ERR_BUF_SIZE);

		if(bytes_written > ERR_BUF_SIZE) {
			free(errbuf);
			return new_simple_regex_result_error(TSTR_LIT("error in error allocation"));
		}

		errbuf[bytes_written] = '\0';
		tstr error = tstr_own(errbuf, bytes_written, ERR_BUF_SIZE);

		return new_simple_regex_result_error(error);
	}

	return new_simple_regex_result_ok(result);
}

NODISCARD bool simple_regex_match(const SimpleRegex* const regex, const tstr* const str) {

	if(regex == NULL) {
		return false;
	}

	// define string boundaries in the first pmatch value
	const int execute_flags = REG_STARTEND;

	regmatch_t pmatch[1] = { (regmatch_t){ .rm_so = 0, .rm_eo = tstr_len(str) } };

	int match_result = regexec(&(regex->regex), tstr_cstr(str), 1, pmatch, execute_flags);

	if(match_result == REG_NOMATCH) {
		return false;
	}

	if(match_result == 0) {
		return true;
	}

	// TODO(Totto): maybe log that error
	// an error occurred
	return false;
}

void free_simple_regex(SimpleRegex* const regex) {
	regfree(&(regex->regex));
}
