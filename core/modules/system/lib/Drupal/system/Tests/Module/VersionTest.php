<?php

/**
 * @file
 * Definition of Drupal\system\Tests\Module\VersionTest.
 */

namespace Drupal\system\Tests\Module;

/**
 * Test module dependency on specific versions.
 */
class VersionTest extends ModuleTestBase {

  /**
   * Modules to enable.
   *
   * @var array
   */
  public static $modules = array('module_test');

  public static function getInfo() {
    return array(
      'name' => 'Module versions',
      'description' => 'Check module version dependencies.',
      'group' => 'Module',
    );
  }

  /**
   * Test version dependencies.
   */
  function testModuleVersions() {
    $dependencies = array(
      // Alternating between being compatible and incompatible with 8.x-2.4-beta3.
      // The first is always a compatible.
      'common_test',
      // Branch incompatibility.
      'common_test (1.x)',
      // Branch compatibility.
      'common_test (2.x)',
      // Another branch incompatibility.
      'common_test (>2.x)',
      // Another branch compatibility.
      'common_test (<=2.x)',
      // Another branch incompatibility.
      'common_test (<2.x)',
      // Another branch compatibility.
      'common_test (>=2.x)',
      // Nonsense, misses a dash. Incompatible with everything.
      'common_test (=8.x2.x, >=2.4)',
      // Core version is optional. Compatible.
      'common_test (=8.x-2.x, >=2.4-alpha2)',
      // Test !=, explicitly incompatible.
      'common_test (=2.x, !=2.4-beta3)',
      // Three operations. Compatible.
      'common_test (=2.x, !=2.3, <2.5)',
      // Testing extra version. Incompatible.
      'common_test (<=2.4-beta2)',
      // Testing extra version. Compatible.
      'common_test (>2.4-beta2)',
      // Testing extra version. Incompatible.
      'common_test (>2.4-rc0)',
    );
    state()->set('system_test.dependencies', $dependencies);
    $n = count($dependencies);
    for ($i = 0; $i < $n; $i++) {
      $this->drupalGet('admin/modules');
      $checkbox = $this->xpath('//input[@id="edit-modules-testing-module-test-enable"]');
      $this->assertEqual(!empty($checkbox[0]['disabled']), $i % 2, $dependencies[$i]);
    }
  }
}
