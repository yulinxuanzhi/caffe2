/**
 * Copyright (c) 2016-present, Facebook, Inc.
 *
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

#ifndef CAFFE2_OPERATORS_IF_OP_H_
#define CAFFE2_OPERATORS_IF_OP_H_

#include "caffe2/core/context.h"
#include "caffe2/core/logging.h"
#include "caffe2/core/operator.h"

namespace caffe2 {

template <class Context>
class IfOp final : public Operator<Context> {
 public:
  IfOp(const OperatorDef& operator_def, Workspace* ws)
      : Operator<Context>(operator_def, ws) {
    CAFFE_ENFORCE(
        this->template HasSingleArgumentOfType<NetDef>("then_net"),
        "then_net must be specified in If operator");
    auto then_net_def =
        this->template GetSingleArgument<NetDef>("then_net", NetDef());
    then_net_ = CreateNet(then_net_def, ws);
    CAFFE_ENFORCE(then_net_, "Failed to initialize then subnet");

    if (this->template HasSingleArgumentOfType<NetDef>("else_net")) {
      auto else_net_def =
          this->template GetSingleArgument<NetDef>("else_net", NetDef());
      else_net_ = CreateNet(else_net_def, ws);
      CAFFE_ENFORCE(else_net_, "Failed to initialize else subnet");
    }
  }

  USE_OPERATOR_CONTEXT_FUNCTIONS;

  bool RunOnDevice() override {
    CAFFE_ENFORCE(
        this->template InputIsType<Tensor<Context>>(0),
        "Invalid condition in If operator: tensor expected");

    const auto& condition = Input(0);
    CAFFE_ENFORCE_EQ(
        condition.size(),
        1,
        "Invalid condition tensor in If operator: single value expected");

    auto conditionValue = *condition.template data<bool>();
    if (conditionValue) {
      return then_net_->Run();
    } else if (else_net_) {
      return else_net_->Run();
    }

    return true;
  }

 private:
  std::unique_ptr<NetBase> then_net_;
  std::unique_ptr<NetBase> else_net_;
};

} // namespace caffe2

#endif // CAFFE2_OPERATORS_IF_OP_H_
