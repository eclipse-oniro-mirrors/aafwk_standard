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

#include "new_ability_impl.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
/**
 * @brief Handling the life cycle switching of NewAbility.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 */

void NewAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    APP_LOGI("NewAbilityImpl::HandleAbilityTransaction begin sourceState:%{public}d; targetState: %{public}d; "
             "isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant,
        targetState.sceneFlag);
#ifdef SUPPORT_GRAPHICS
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequsetFocus(want);
        }
        APP_LOGE("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);

    if (ability_ && lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
        ability_->SetStartAbilitySetting(targetState.setting);
        ability_->SetLaunchParam(targetState.launchParam);
        Start(want);
        CheckAndRestore();
    }
#ifdef SUPPORT_GRAPHICS
    if (ability_ != nullptr) {
        ability_->sceneFlag_ = targetState.sceneFlag;
    }
#endif
    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        APP_LOGI("AbilityThread::HandleAbilityTransaction before AbilityManagerClient->AbilityTransitionDone");
        AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
        APP_LOGI("AbilityThread::HandleAbilityTransaction after AbilityManagerClient->AbilityTransitionDone");
    }
    APP_LOGI("NewAbilityImpl::HandleAbilityTransaction end");
}

/**
 * @brief Handling the life cycle switching of NewAbility in switch.
 *
 * @param want Indicates the structure containing information about the ability.
 * @param targetState The life cycle state to switch to.
 *
 * @return return true if need notify ams, otherwise return false.
 *
 */
bool NewAbilityImpl::AbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    APP_LOGI("NewAbilityImpl::AbilityTransaction begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
#ifdef SUPPORT_GRAPHICS
            if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
                Background();
            }
#endif
            Stop();
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (targetState.isNewWant) {
                NewWant(want);
            }
#ifdef SUPPORT_GRAPHICS
            Foreground(want);
#endif
            ret = false;
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
#ifdef SUPPORT_GRAPHICS
            Background();
#endif
            break;
        }
        default: {
            ret = false;
            APP_LOGE("NewAbilityImpl::HandleAbilityTransaction state error");
            break;
        }
    }
    APP_LOGI("NewAbilityImpl::AbilityTransaction end: retVal = %{public}d", (int)ret);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
