<?php
/**
 * @file
 * Admin callbacks for the whatthefeature module.
 */

/**
 * Report page
 */
function wtf_admin_components_form($form, &$form_state) {
  if (isset($form_state['triggering_element'])) {
    if ($form_state['triggering_element']['#name'] == 'ct_key') {
      $form_state['storage']['ct_key'] = $form_state['values']['ct_key'] == '_none' ? NULL : $form_state['values']['ct_key'];
      $form_state['storage']['module_key'] = NULL;
    }
    elseif ($form_state['triggering_element']['#name'] == 'module_key') {
      $form_state['storage']['module_key'] = $form_state['values']['module_key'] == '_none' ? NULL : $form_state['values']['module_key'];
    }
  }

  if (!isset($form_state['storage']['ct_options'])) {
    $all_components = features_get_component_map();
    $ctypes = features_get_components();
    $options = array('_none' => t('- Select -'));
    foreach ($ctypes as $key => $ct) {
      if (!isset($all_components[$key])) {
        continue;
      }
      // Initialize by setting the first component type as active
      if (!isset($form_state['storage']['ct_key'])) {
        $form_state['storage']['ct_key'] = $key;
      }
      $options[$key] = $ct['name'] . ' (' . count($all_components[$key]) . ')';
    }
    $form_state['storage']['ct_options'] = $options;
  }

  $form['ct_key'] = array(
    '#type' => 'select',
    '#title' => t('Component type'),
    '#options' => $form_state['storage']['ct_options'],
    '#default_value' => $form_state['storage']['ct_key'],
    '#ajax' => array(
      'callback' => 'wtf_ajax_admin_components_form_components',
      'wrapper' => 'components_replace'
    )
  );

  if (!isset($form_state['storage']['modules'])) {
    $modules = features_get_modules();
    foreach ($modules as $module) {
      $options[$module->name] = $module->info['name'];
    }
    $form_state['storage']['modules'] = $options;
  }

  $form['components'] = array(
    '#prefix' => '<div id="components_replace">',
    '#suffix' => '</div>'
  );

  if (isset($form_state['storage']['ct_key'])) {
    $components = features_get_component_map($form_state['storage']['ct_key']);
    $last_ct = NULL;
    $header = array(
      '',
      t('Component name'),
      t('Features module')
    );
    $rows = array();
    $filtered_modules = array('_none' => t('- All - '));
    $i = 1;
    foreach ($components as $c_name => $c_modules) {
      $modules = array();
      if (is_array($c_modules)) {
        foreach ($c_modules as $m) {
          // Only use modules with data returned from features_get_modules
          if (!isset($form_state['storage']['modules'][$m])) {
            continue;
          }
          $modules[$m] = l($form_state['storage']['modules'][$m], 'admin/structure/features/' . $m);
          $filtered_modules[$m] = $form_state['storage']['modules'][$m];
        }
      }
      // Skip if no module is associated
      if (empty($modules)) {
        continue;
      }
      // Skip if a module filter is selected, but it doesn't exist in this component
      if (isset($form_state['storage']['module_key']) && !array_key_exists($form_state['storage']['module_key'], $modules)) {
        continue;
      }

      $rows[] = array(
        $i . '.',
        $c_name,
        !empty($modules) ? theme('item_list', array('items' => $modules)) : ''
      );
      $i++;
    }

    $form['components']['module_key'] = array(
      '#type' => 'select',
      '#title' => t('Filter by module'),
      '#options' => $filtered_modules,
      '#ajax' => array(
        'callback' => 'wtf_ajax_admin_components_form_components',
        'wrapper' => 'components_replace'
      )
    );

    $form['components']['component_table'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows
    );
  }

  return $form;
}

function wtf_admin_components_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/*
 * AJAX callback for the component type selector.
 */
function wtf_ajax_admin_components_form_components($form, &$form_state) {
  return $form['components'];
}