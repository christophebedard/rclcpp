// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>
#include <memory>

#include "rclcpp/strategies/allocator_memory_strategy.hpp"
#include "rclcpp/memory_strategy.hpp"
#include "rclcpp/node.hpp"
#include "rclcpp/rclcpp.hpp"

class TestFindWeakNodes : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  {
    rclcpp::init(0, nullptr);
  }

  static void TearDownTestCase()
  {
    rclcpp::shutdown();
  }
};

TEST_F(TestFindWeakNodes, allocator_strategy_with_weak_nodes) {
  // GIVEN
  // A vector of weak pointers to nodes
  auto memory_strategy = std::make_shared<
    rclcpp::memory_strategies::allocator_memory_strategy::AllocatorMemoryStrategy<>>();
  auto existing_node = rclcpp::Node::make_shared("existing_node");
  auto dead_node = rclcpp::Node::make_shared("dead_node");
  auto existing_group = existing_node->get_node_base_interface()->get_default_callback_group();
  auto dead_group = dead_node->get_node_base_interface()->get_default_callback_group();
  rclcpp::memory_strategy::MemoryStrategy::WeakCallbackGroupsToNodesMap weak_groups_to_nodes;
  weak_groups_to_nodes[existing_group] = existing_node->get_node_base_interface();
  weak_groups_to_nodes[dead_group] = dead_node->get_node_base_interface();

  // AND
  // Delete dead_node, creating a dangling pointer in weak_nodes
  dead_node.reset();
  ASSERT_FALSE(weak_groups_to_nodes[existing_group].expired());
  ASSERT_TRUE(weak_groups_to_nodes[dead_group].expired());

  // WHEN
  bool has_invalid_weak_groups_or_nodes = memory_strategy->collect_entities(weak_groups_to_nodes);

  // THEN
  // The result of finding dangling node pointers should be true
  ASSERT_TRUE(has_invalid_weak_groups_or_nodes);

  // Prevent memory leak due to the order of destruction
  memory_strategy->clear_handles();
}

TEST_F(TestFindWeakNodes, allocator_strategy_no_weak_nodes) {
  // GIVEN
  // A vector of weak pointers to nodes, all valid
  auto memory_strategy = std::make_shared<
    rclcpp::memory_strategies::allocator_memory_strategy::AllocatorMemoryStrategy<>>();
  auto existing_node1 = rclcpp::Node::make_shared("existing_node1");
  auto existing_node2 = rclcpp::Node::make_shared("existing_node2");
  auto existing_group1 = existing_node1->get_node_base_interface()->get_default_callback_group();
  auto existing_group2 = existing_node2->get_node_base_interface()->get_default_callback_group();
  rclcpp::memory_strategy::MemoryStrategy::WeakCallbackGroupsToNodesMap weak_groups_to_nodes;
  weak_groups_to_nodes[existing_group1] = existing_node1->get_node_base_interface();
  weak_groups_to_nodes[existing_group2] = existing_node2->get_node_base_interface();
  ASSERT_FALSE(weak_groups_to_nodes[existing_group1].expired());
  ASSERT_FALSE(weak_groups_to_nodes[existing_group2].expired());

  // WHEN
  bool has_invalid_weak_groups_or_nodes = memory_strategy->collect_entities(weak_groups_to_nodes);

  // THEN
  // The result of finding dangling node pointers should be false
  ASSERT_FALSE(has_invalid_weak_groups_or_nodes);

  // Prevent memory leak due to the order of destruction
  memory_strategy->clear_handles();
}
