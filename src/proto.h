#ifndef __PROTO_H__
#define __PROTO_H__

/**
 * @file proto.h
 * @brief Simple string-based protocol for arithmetic and string operations.
 *
 * Protocol Syntax:
 * @code
 * <opcode> <operand1>[,<operand2>]
 * @endcode
 *
 * Supported Opcodes:
 * - ADD: Add two numbers.
 * - SUB: Subtract two numbers.
 * - MUL: Multiply two numbers.
 * - DIV: Divide operand1 by operand2.
 * - LEN: Calculate the length of a string (uses operand1).
 * - CAT: Concatenate operand1 and operand2.
 * - FIND: Find the starting index of substring (operand1) within operand2.
 */

/**
 * @brief Processes a protocol request and returns a status-prefixed response.
 *
 * Parses the input request, executes the corresponding logic, and populates
 * the response buffer. All successful responses start with "[OK]", while
 * failures start with "[ERROR]".
 *
 * @param[in]  req     Input buffer containing the request string.
 * @param[out] resp    Output buffer for the response string.
 * @param[in]  sz_resp The allocated size of the response buffer.
 *
 * @return int The total length of the response string written to @p resp.
 */
int proto_exec(char *req, char *resp, int sz_resp);

#endif
