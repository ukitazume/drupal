<?php

/**
 * @file
 * Definition of Drupal\views\Plugin\Type\DefaultWizardDeriver.
 */

namespace Drupal\views\Plugin\Type;

use Drupal\Component\Plugin\Derivative\DerivativeInterface;

/**
 * A derivative class which provides automatic wizards for all base tables.
 */
class DefaultWizardDeriver implements DerivativeInterface {
  /**
   * Stores all base table plugin information.
   *
   * @var array
   */
  protected $derivatives = array();

  /**
   * Implements Drupal\Component\Plugin\Derivative\DerivativeInterface::getDerivativeDefinition().
   */
  public function getDerivativeDefinition($derivative_id, array $base_plugin_definition) {
    if (!empty($this->derivatives) && !empty($this->derivatives[$derivative_id])) {
      return $this->derivatives[$derivative_id];
    }
    $this->getDerivativeDefinitions($base_plugin_definition);
    return $this->derivatives[$derivative_id];
  }

  /**
   * Implements Drupal\Component\Plugin\Derivative\DerivativeInterface::getDerivativeDefinitions().
   */
  public function getDerivativeDefinitions(array $base_plugin_definition) {
    $base_tables = array_keys(views_fetch_base_tables());
    $this->derivatives = array();
    foreach ($base_tables as $table) {
      $views_info = views_fetch_data($table);
      if (empty($views_info['table']['wizard_id'])) {
        $this->derivatives[$table] = array(
          'id' => 'standard',
          'base_table' => $table,
          'title' => $views_info['table']['base']['title'],
          'class' => 'Drupal\views\Plugin\views\wizard\Standard'
        );
      }
    }
    return $this->derivatives;

  }

}
