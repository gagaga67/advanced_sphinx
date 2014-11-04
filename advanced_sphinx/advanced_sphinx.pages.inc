<?php
// $Id: advanced_sphinx.pages.inc, v 1.0 2011/07/05 19:59:16 gagaga Exp $
/**
 * @file
 * Implementation of pages used to search using Sphinx search module.
 */

function advanced_sphinx_search_page() {
  global $user;
  // Parse request and build search options structure.
  $search_options = advanced_sphinx_parse_request($_GET);
  variable_set('advanced_sphinx_keys', $_GET['keys']);

  // Execute search query and collect the results.
  if ($search_options['keys'] || (!$search_options['keys'] && $search_options['view'])) {
    $search_results = advanced_sphinx_execute_query($search_options);
    $output = drupal_render(drupal_get_form('advanced_sphinx_search_form', $search_options));
    $output .= theme('advanced_sphinx_search_results', array('search_options'=>$search_options, 'search_results'=>$search_results));
  }
  else{
    $output = drupal_render(drupal_get_form('advanced_sphinx_search_form', $search_options));
  }
  return $output;
}

/**
 * Record a warning to watchdog.
 *
 * If current user has 'administer advanced_sphinx' privilege,
 * then the warning is just sent to the user interface.
 *
 * @param string $message
 */
function advanced_sphinx_watchdog_warning($message) {
  if (user_access('administer advanced_sphinx')) {
    drupal_set_message(check_plain($message), 'error');
  }
  else {
    watchdog('advanced_sphinx', $message, NULL, WATCHDOG_WARNING);
  }
}

/**
 * Render a search form.
 *
 * @param array $search_options
 *   The search string and options entered by the user.
 * @param boolean $advanced_options_collapsed
 *   TRUE to collapse Advanced search options fieldset.
 * @return array
 *   The search form.
 */
function advanced_sphinx_search_form($form, &$form_state, $search_options = array(), $advanced_options_collapsed = TRUE) {
  #drupal_set_message('<pre>' . print_r($search_options, 1) . '</pre>');
  $form = array(
    '#action' => url(variable_get('advanced_sphinx_search_path', 'search-content')),
    '#attributes' => array(
      'class' => 'search-form'
    ),
  );
  $form['basic'] = array(
    '#type' => 'item',
    '#title' => t('Enter your keywords')
  );
  $form['basic']['inline'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>'
  );
  $form['basic']['inline']['keys'] = array(
    '#type' => 'textfield',
    '#title' => '',
    '#default_value' => $search_options['keys'],
    '#size' => 50,
    '#maxlength' => 255,
  );
  $form['basic']['inline']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Search')
  );

  if (variable_get('advanced_sphinx_use_adv_form', FALSE) != 0) {
    $form['basic']['filters'] = array(
      '#type' => 'fieldset',
      '#title' => t('Advanced search'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $types_node_use = variable_get('advanced_sphinx_type_filter', array());
    $query = db_select('node_type', 'n');
    $result = $query
      ->fields('n', array('type', 'name'))
      ->execute()
      ->fetchAll();
    foreach ($result as $res) {
      $crc = sprintf("%u", crc32($res->type));
      if (count($types_node_use) > 0) {
        if (isset($types_node_use[$crc])) {
          $opts[$crc] = $res->name;
        }
      }
      else {
        $opts[$crc] = $res->name;
      }
    }

    if (isset($search_options['filters']['type']) && !empty($search_options['filters']['type'])) {
      $res_type = $search_options['filters']['type'];
    }
    else {
      $res_type = array();
    }

    $form['basic']['filters']['node_type'] = array(
      '#type' => 'select',
      '#title' => t('Type of documents'),
      '#default_value' => $res_type,
      '#options' => $opts,
      '#multiple' => TRUE,
      '#size' => 10,
      '#prefix' => '<div class="container-inline">',
    );

    if (isset($search_options['filters']['language']) && !empty($search_options['filters']['language'])) {
      $res_language = $search_options['filters']['language'];
    }
    else {
      $res_language = array();
    }

    $lang_arr = language_list();
    foreach ($lang_arr as $kl => $vl) {
      $crc = sprintf("%u", crc32($kl));
      //drupal_set_message('<pre>' . print_r(array($kl, crc32($kl), sprintf("%u", crc32($kl)), $vl->native), 1) . '</pre>');
      $langopts[$crc] = $vl->native;
    }
    $form['basic']['filters']['language'] = array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#default_value' => $res_language,
      '#options' => $langopts,
      '#multiple' => TRUE,
      '#size' => 10,
      '#suffix' => '</div>',
    );
  }
  return $form;
}

/**
 * Validate a search form submission.
 */
function advanced_sphinx_search_form_validate($form, &$form_state) {
  if (empty($form_state['values']['keys'])) {
      form_set_error('keys', t('Enter a search keys in the field!'));
  }
}

/**
 * Process a search form submission.
 */
function advanced_sphinx_search_form_submit($form, &$form_state) {
  $search_options = advanced_sphinx_parse_request($form_state['values']);
  $query = advanced_sphinx_get_query_string($search_options);
  if (empty($query)) {
    form_set_error('keys', t('Please enter some keywords and/or other search options.'));
  }
  // Transform POST into a GET request.
  advanced_sphinx_search_goto($query);
}

function theme_advanced_sphinx_search_results($variables = array()) {
  $search_options = $variables['search_options'];
  $search_results = $variables['search_results'];
  if (!isset($variables['result_wrapper']) || (!isset($result_wrapper['result']) || empty($result_wrapper['result']))) {
    $result_wrapper = advanced_sphinx_get_items_result_main($search_options, $search_results);
  }
  else {
    $result_wrapper = $variables['result_wrapper'];
  }
  $output = '';
  $result_wrapper['stat'] = format_plural($search_results['total_found'], 'Нашёлся 1 ответ', 'Нашлось @count ответов') . ' ' . t('за @seconds секунд.', array('@seconds' => round($search_results['time'], 3)));
  if (!empty($search_results['words'])) {
    $words = array();
    foreach ($search_results['words'] as $word => $word_data) {
      $words[] = '<em>' . check_plain($word) . '</em> (' . t('ответов: @docs, повторений: @hits', array('@docs' => (int)$word_data['docs'], '@hits' => (int)$word_data['hits'])) . ')';
    }
    $result_wrapper['word_stat'] = implode('; ', $words) . '.';
  }
  
  $result_wrapper['sorted'] = theme('advanced_sphinx_sorted_links', array('query' => advanced_sphinx_get_query_string($search_options)));
  $output .= theme('advanced_sphinx_result_wrapper', array('result' => $result_wrapper));
  // Display pager.
  $output .= advanced_sphinx_pager($search_results['total_available'], $search_options['results_per_page']);
  return $output;
}

function theme_advanced_sphinx_sorted_links($variables) {
  $query = $variables['query'];
  $sort = $variables['sort'];
  $sort_variant = array(
    'sor-relevance',
    'sor-created',
    'sor-changed'
  );
  $sort_title = array(
    t('Relevance'),
    t('Date'),
    t('Updated')
  );
  $url = array();
  $sorted = '';
  foreach ($query as $sort => $sval) {
    $i = 0;
    if (strstr($sort, 'sor-') != FALSE) {
      unset($query[$sort]);
      while ($i < 3) {
        $query[$sort_variant[$i]] = 'DESC';
        $class = NULL;
        if ($sort == $sort_variant[$i]) {
          $class = 'class="sortnow"';
        }
        $url[] = '<a id="' . $sort_variant[$i] . '" href="' . url(variable_get('advanced_sphinx_search_path', 'search-content'), array('absolute' => TRUE, 'query' => $query)) . '" ' . $class . ' >' . $sort_title[$i] . '</a>';
        unset($query[$sort_variant[$i]]);
        $i++;
      }
      $key = str_replace("sor-", "", $sort);
      $sorted = $key;
    }
  }
  if ($sorted == '') {
    $i = 0;
    while ($i < 3) {
      $query[$sort_variant[$i]] = 'DESC';
      $class = NULL;
      if ('sor-relevance' == $sort_variant[$i]) {
        $class = 'class="sortnow"';
      }
      $url[] = '<a id="' . $sort_variant[$i] . '" href="' . url(variable_get('advanced_sphinx_search_path', 'search-content'), array('absolute' => TRUE, 'query' => $query)) . '" ' . $class . ' >' . $sort_title[$i] . '</a>';
      unset($query[$sort_variant[$i]]);
      $i++;
    }
  }
  return implode('  ', $url);
}

function advanced_sphinx_get_items_result_main($search_options, $search_results) {
  $items_result = array();
  $result_wrapper = array();
  $result_wrapper['result'] = '';
  $i = 1;
  foreach ($search_results['nodes'] as $item_id => $node) {
    $items_result['number'] = isset($_GET['page']) ? $i + $_GET['page'] * $search_options['results_per_page'] : $i;
    $title = (isset($search_results['titles'][$item_id]) ? $search_results['titles'][$item_id] : check_plain($node->title));
    if (variable_get('advanced_sphinx_logs', TRUE)) {
      $items_result['title'] = l($title, variable_get('advanced_sphinx_search_path', 'search-content') . '/service/' . $node->nid, array('html' => TRUE));
    }
    else {
      $items_result['title'] = l($title, 'node/' . $node->nid, array('html' => TRUE));
    }
    $items_result['excerpts'] = isset($search_results['excerpts'][$item_id]) ? '<p class="search-excerpt">' . $search_results['excerpts'][$item_id] . '</p>' . "\n" : '';
    //drupal_set_message('<pre>' . print_r($search_results['excerpts'], 1) . '</pre>');
    //drupal_set_message('<pre>' . print_r($items_result['excerpts'], 1) . '</pre>');
    if (module_exists('taxonomy') && is_array($node->taxonomy)) {
      $links = array();
      foreach ($node->taxonomy as $term) {
        //$links[] = l($term->name, taxonomy_term_path($term));
      }
      $items_result['tax'] = implode(' - ', $links);
    }
    $items_result['username'] = theme('username', array('account' => $node));
    $items_result['type'] = $node->type;
    $items_result['date'] = $node->created;
    if (isset($node->files_count)) {
      $items_result['file'] = $node->files_count;
    }
    if (isset($node->comment_count)) {
      $items_result['comment_count'] = $node->comment_count;
    }
    $result_wrapper['result'] .= theme('advanced_sphinx_items_result_main', array('result' => $items_result));
  $i++;
  }
  return $result_wrapper;
}
