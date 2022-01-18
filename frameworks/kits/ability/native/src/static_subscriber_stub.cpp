/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "static_subscriber_stub.h"

namespace OHOS {
namespace AppExecFwk {
int StaticSubscriberStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    switch (code) {
        case COMMAND_ON_COMMON_EVENT_TRIGGERED: {
            CommonEventData* inData = data.ReadParcelable<CommonEventData>();
            ErrCode ec = OnCommonEventTriggered(inData);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
} // namespace AppExecFwk
} // namespace OHOS