/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "call_record.h"

#include "hilog_wrapper.h"
#include "ability_util.h"
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "ability_record.h"
#include "element_name.h"

namespace OHOS {
namespace AAFwk {
int64_t CallRecord::callRecordId = 0;

CallRecord::CallRecord(const int32_t callerUid, const std::shared_ptr<AbilityRecord> &targetService,
    const sptr<IAbilityConnection> &connCallback, const sptr<IRemoteObject> &callToken)
    : callerUid_(callerUid),
      state_(CallState::INIT),
      service_(targetService),
      connCallback_(connCallback),
      callerToken_(callToken)
{
    recordId_ = callRecordId++;
    startTime_ = AbilityUtil::SystemTimeMillis();
}

CallRecord::~CallRecord()
{
    if (callRemoteObject_ && callDeathRecipient_) {
        callRemoteObject_->RemoveDeathRecipient(callDeathRecipient_);
    }
}

std::shared_ptr<CallRecord> CallRecord::CreateCallRecord(const int32_t callerUid,
    const std::shared_ptr<AbilityRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
    const sptr<IRemoteObject> &callToken)
{
    auto callRecord = std::make_shared<CallRecord>(callerUid, targetService, connCallback, callToken);
    CHECK_POINTER_AND_RETURN(callRecord, nullptr);
    callRecord->SetCallState(CallState::INIT);
    return callRecord;
}

void CallRecord::SetCallStub(const sptr<IRemoteObject> & call)
{
    CHECK_POINTER(call);

    callRemoteObject_ = call;

    HILOG_DEBUG("SetCallStub complete.");

    if (callDeathRecipient_ == nullptr) {
        callDeathRecipient_ =
                new AbilityCallRecipient(std::bind(&CallRecord::OnCallStubDied, this, std::placeholders::_1));
    }

    callRemoteObject_->AddDeathRecipient(callDeathRecipient_);
}

sptr<IRemoteObject> CallRecord::GetCallStub()
{
    return callRemoteObject_;
}

void CallRecord::SetConCallBack(const sptr<IAbilityConnection> &connCallback)
{
    connCallback_ = connCallback;
}

sptr<IAbilityConnection> CallRecord::GetConCallBack() const
{
    return connCallback_;
}

AppExecFwk::ElementName CallRecord::GetTargetServiceName() const
{
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    if (tmpService) {
        const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
        AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name);
        return element;
    }
    return AppExecFwk::ElementName();
}

sptr<IRemoteObject> CallRecord::GetCallerToken() const
{
    return callerToken_;
}

bool CallRecord::SchedulerConnectDone()
{
    HILOG_DEBUG("Scheduler Connect Done by callback. id:%{public}d", recordId_);
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    if (!callRemoteObject_ || !connCallback_ || !tmpService) {
        HILOG_ERROR("callstub or connCallback is nullptr, can't scheduler connect done.");
        return false;
    }

    const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name);
    connCallback_->OnAbilityConnectDone(element, callRemoteObject_, ERR_OK);
    state_ = CallState::REQUESTED;

    HILOG_DEBUG("element: %{public}s, result: %{public}d. connectstate:%{public}d.", element.GetURI().c_str(),
        ERR_OK, state_);
    return true;
}

bool CallRecord::SchedulerDisConnectDone()
{
    HILOG_DEBUG("Scheduler disconnect Done by callback. id:%{public}d", recordId_);
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    if (!connCallback_ || !tmpService) {
        HILOG_ERROR("callstub or connCallback is nullptr, can't scheduler connect done.");
        return false;
    }

    const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name);
    connCallback_->OnAbilityDisconnectDone(element,  ERR_OK);
    
    return true;
}

void CallRecord::OnCallStubDied(const wptr<IRemoteObject> & remote)
{
    HILOG_WARN("callstub is died. id:%{public}d", recordId_);
    auto object = remote.promote();
    CHECK_POINTER(object);

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);
    auto task = [abilityManagerService, callRecord = shared_from_this()]() {
        abilityManagerService->OnCallConnectDied(callRecord);
    };
    handler->PostTask(task);
}

void CallRecord::Dump(std::vector<std::string> &info) const
{
    HILOG_DEBUG("CallRecord::Dump is called");
}

int32_t CallRecord::GetCallerUid() const
{
    return callerUid_;
}

bool CallRecord::IsCallState(const CallState& state) const
{
    return (state_ == state);
}

void CallRecord::SetCallState(const CallState& state)
{
    state_ = state;
}

int CallRecord::GetCallRecordId() const
{
    return recordId_;
}
}  // namespace AAFwk
}  // namespace OHOS