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

#include "ability_context_impl.h"

#include "ability_manager_client.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::string AbilityContextImpl::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContextImpl::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContextImpl::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContextImpl::GetStorageDir()
{
    return stageContext_ ? stageContext_->GetStorageDir() : "";
}

std::string AbilityContextImpl::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContextImpl::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string AbilityContextImpl::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want &want, int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbility. Start calling ams->StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling ams->StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions,
    int requestCode)
{
    HILOG_DEBUG("AbilityContextImpl::StartAbility. Start calling ams->StartAbility.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    HILOG_INFO("AbilityContextImpl::StartAbility. End calling ams->StartAbility. ret=%{public}d", err);
    return err;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    HILOG_DEBUG("%{public}s. Start calling ams->StartAbilityForResult.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    HILOG_INFO("%{public}s. End calling ams->StartAbilityForResult. ret=%{public}d", __func__, err);
    return err;
}


ErrCode AbilityContextImpl::TerminateAbilityWithResult(const AAFwk::Want &want, int resultCode)
{
    HILOG_DEBUG("%{public}s. Start calling ams->TerminateAbilityWithResult.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, resultCode, &want);
    HILOG_INFO("%{public}s. End calling ams->TerminateAbilityWithResult. ret=%{public}d", __func__, err);
    return err;
}

void AbilityContextImpl::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    HILOG_DEBUG("%{public}s. Start calling ams->OnAbilityResult.", __func__);
    resultCallbacks_[requestCode](resultCode, resultData);
    resultCallbacks_.erase(requestCode);
    HILOG_INFO("%{public}s. End calling ams->OnAbilityResult.", __func__);
}

bool AbilityContextImpl::ConnectAbility(const AAFwk::Want &want,
                                        const std::shared_ptr<AbilityConnectCallback> &connectCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("AbilityContextImpl::ConnectAbility ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

void AbilityContextImpl::DisconnectAbility(const AAFwk::Want &want,
                                           const std::shared_ptr<AbilityConnectCallback> &connectCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end ams->DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    HILOG_INFO("%{public}s end ams->DisconnectAbility", __func__);
}

std::string AbilityContextImpl::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContextImpl::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::string AbilityContextImpl::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContextImpl::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContextImpl::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

std::shared_ptr<Context> AbilityContextImpl::CreateBundleContext(const std::string &bundleName)
{
    return stageContext_ ? stageContext_->CreateBundleContext(bundleName) : nullptr;
}

void AbilityContextImpl::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContextImpl::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContextImpl::SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    stageContext_ = stageContext;
}

void AbilityContextImpl::MinimizeAbility()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->MinimizeAbility(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContext::MinimizeAbility is failed %{public}d", err);
    }
    HILOG_INFO("%{public}s end.", __func__);
}

ErrCode AbilityContextImpl::TerminateSelf()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    AAFwk::Want resultWant;
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, &resultWant);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityContextImpl::TerminateSelf is failed %{public}d", err);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

sptr<IRemoteObject> AbilityContextImpl::GetAbilityToken()
{
    return token_;
}

void AbilityContextImpl::RequestPermissionsFromUser(const std::vector<std::string> &permissions, int requestCode)
{
    HILOG_INFO("%{public}s called.", __func__);
}

ErrCode AbilityContextImpl::RestoreWindowStage(void* contentStorage)
{
    HILOG_INFO("%{public}s begin. contentStorage = %{public}p", __func__, contentStorage);
    ErrCode err = ERR_OK;
    contentStorage_ = contentStorage;
    return err;
}
}  // namespace AbilityRuntime
}  // namespace OHOS