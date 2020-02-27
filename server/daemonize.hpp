#ifndef CLDCB_SERVER_DAEMONIZE_HPP
#define CLDCB_SERVER_DAEMONIZE_HPP

#include<functional>

/* Daemonize the process:
 * The given handler will be executed in the child (daemon)
 * process.
 * It is given a single argument, a function which when invoked,
 * will cause the original daemonize function to return with
 * the given argument.
 * The intent is that the child will first do some basic
 * initializations, then if the initializations are okay the
 * handler will call the given function and let the original
 * process exit.
 * If initializations are bad, the child can print to
 * stderr and then signal a failure to the original process
 * (i.e. call the given function with non-zero argument).
 *
 * The handler can still use stdin stdout stderr, but once
 * it has called the given argument, all of those are closed
 * and will be redirected to /dev/null.
 *
 * The handler *has to* call the given argument or else it
 * will remain a child of the parent and the parent will
 * not exit.
 */
int daemonize(std::function<void (std::function<void (int)>)> handler);

#endif /* CLDCB_SERVER_DAEMONIZE_HPP */
