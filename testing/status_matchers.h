#ifndef RLWE_TESTING_STATUS_MATCHERS_H_
#define RLWE_TESTING_STATUS_MATCHERS_H_

#include <gmock/gmock.h>
#include "absl/status/status.h"
#include "statusor.h"

namespace rlwe {
namespace testing {

namespace internal {

// This function and its overload allow the same matcher to be used for Status
// and StatusOr tests.
absl::Status GetStatus(const absl::Status& status) { return status; }

template <typename T>
absl::Status GetStatus(const rlwe::StatusOr<T>& statusor) {
  return statusor.status();
}

template <typename StatusType>
class StatusIsImpl : public ::testing::MatcherInterface<StatusType> {
 public:
  StatusIsImpl(const ::testing::Matcher<absl::StatusCode>& code,
               const ::testing::Matcher<const std::string&>& message)
      : code_(code), message_(message) {}

  bool MatchAndExplain(StatusType status,
                       ::testing::MatchResultListener* listener) const {
    ::testing::StringMatchResultListener str_listener;
    absl::Status real_status = GetStatus(status);
    if (!code_.MatchAndExplain(real_status.code(), &str_listener)) {
      *listener << str_listener.str();
      return false;
    }
    if (!message_.MatchAndExplain(
            static_cast<std::string>(real_status.message()), &str_listener)) {
      *listener << str_listener.str();
      return false;
    }
    return true;
  }

  void DescribeTo(std::ostream* os) const {
    *os << "has a status code that ";
    code_.DescribeTo(os);
    *os << " and a message that ";
    message_.DescribeTo(os);
  }

  void DescribeNegationto(std::ostream* os) const {
    *os << "has a status code that ";
    code_.DescribeNegationTo(os);
    *os << " and a message that ";
    message_.DescribeNegationTo(os);
  }

 private:
  ::testing::Matcher<absl::StatusCode> code_;
  ::testing::Matcher<const std::string&> message_;
};

class StatusIsPoly {
 public:
  StatusIsPoly(::testing::Matcher<absl::StatusCode>&& code,
               ::testing::Matcher<const std::string&>&& message)
      : code_(code), message_(message) {}

  // Converts this polymorphic matcher to a monomorphic matcher.
  template <typename StatusType>
  operator ::testing::Matcher<StatusType>() const {
    return ::testing::Matcher<StatusType>(
        new StatusIsImpl<StatusType>(code_, message_));
  }

 private:
  ::testing::Matcher<absl::StatusCode> code_;
  ::testing::Matcher<const std::string&> message_;
};

}  // namespace internal

// This function allows us to avoid a template parameter when writing tests, so
// that we can transparently test both Status and StatusOr returns.
internal::StatusIsPoly StatusIs(
    ::testing::Matcher<absl::StatusCode>&& code,
    ::testing::Matcher<const std::string&>&& message) {
  return internal::StatusIsPoly(
      std::forward<::testing::Matcher<absl::StatusCode>>(code),
      std::forward<::testing::Matcher<const std::string&>>(message));
}

}  // namespace testing
}  // namespace rlwe

#endif  // RLWE_TESTING_STATUS_MATCHERS_H_
