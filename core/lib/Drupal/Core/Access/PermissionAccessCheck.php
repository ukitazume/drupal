<?php

/**
 * @file
 * Contains Drupal\Core\Access\PermissionAccessCheck.
 */

namespace Drupal\Core\Access;

use Symfony\Component\Routing\Route;
use Symfony\Component\HttpFoundation\Request;

/**
 * Determines access to routes based on permissions defined via hook_permission().
 */
class PermissionAccessCheck implements AccessCheckInterface {

  /**
   * Implements AccessCheckInterface::applies().
   */
  public function applies(Route $route) {
    return array_key_exists('_permission', $route->getRequirements());
  }

  /**
   * Implements AccessCheckInterface::access().
   */
  public function access(Route $route, Request $request) {
    $permission = $route->getRequirement('_permission');
    // @todo Replace user_access() with a correctly injected and session-using
    //   alternative.
    // If user_access() fails, return NULL to give other checks a chance.
    return user_access($permission) ? TRUE : NULL;
  }
}
