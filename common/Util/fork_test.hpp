#ifndef CLDCB_COMMON_UTIL_FORK_TEST_HPP
#define CLDCB_COMMON_UTIL_FORK_TEST_HPP

#include<functional>
#include<memory>

namespace Util {

/* Fork, then run the given function.
 *
 * If it returns nullptr, this is the parent and the
 * child function returned successfully.
 *
 * If it returns non-nullptr, then:
 * * If it points to a 0, this is the child and we
 *   should exit with a 0 exit code.
 * * If it points to a non-zero, this is the parent
 *   and the child failed.
 */
std::unique_ptr<int> fork_test(std::function<void()> child);

}

#endif /* CLDCB_COMMON_UTIL_FORK_TEST_HPP */
