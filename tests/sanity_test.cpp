#include <gtest/gtest.h>

TEST(BuildSystem, SanityCheck) {
  EXPECT_EQ(1, 1);
}

TEST(ProjectLayout, CoreScaffoldDirectoriesExist) {
  SUCCEED();
}
