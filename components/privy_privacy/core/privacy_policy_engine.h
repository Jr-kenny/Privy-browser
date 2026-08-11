#ifndef COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_POLICY_ENGINE_H_
#define COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_POLICY_ENGINE_H_

#include "components/privy_privacy/core/privacy_types.h"

namespace privy {

// Pure policy object. It performs no IO, owns no browser state, and has no Aleo
// dependency. That makes privacy behavior independently testable.
class PrivacyPolicyEngine {
 public:
  PrivacyPolicyEngine() = default;
  PrivacyPolicyEngine(const PrivacyPolicyEngine&) = delete;
  PrivacyPolicyEngine& operator=(const PrivacyPolicyEngine&) = delete;
  ~PrivacyPolicyEngine() = default;

  PrivacyDecision Evaluate(const PrivacyRequest& request) const;
};

}  // namespace privy

#endif  // COMPONENTS_PRIVY_PRIVACY_CORE_PRIVACY_POLICY_ENGINE_H_
