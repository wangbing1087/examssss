<?php

/**
 * @file
 * Administrative page callbacks for the Exam Spider module.
 */

/**
 * Exam Spider dashboard page callbacks.
 */
function exam_spider_dashboard() {
  $output = NULL;
  $output .= l(t('+ Create @examSpiderExamTitle', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/add', array('html' => TRUE));
  $header = array(
    array(
      'data'  => EXAM_SPIDER_EXAM_TITLE . ' Id',
      'field' => 'id',
      'sort'  => 'desc',
    ),
    array('data' => EXAM_SPIDER_EXAM_TITLE . ' Name', 'field' => 'exam_name'),
    array(
      'data'  => EXAM_SPIDER_EXAM_TITLE . ' Description',
      'field' => 'exam_description',
    ),
    array('data' => "Created By", 'field' => 'uid'),
    array('data' => "Status", 'field' => 'status'),
    array('data' => "Operations"),

  );
  $query = db_select("exam_list", "a")
    ->extend('PagerDefault')
    ->limit(10)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields("a", array(
      'id',
      'exam_name',
      'exam_description',
      'uid',
      'status',
    ));
  $results = $query->execute();
  $rows = array();
  $i = 0;
  foreach ($results as $row) {
    if ($row->status == 0) {
      $status = 'Closed';
    }
    else {
      $status = 'Open';
    }
    $user_data = db_select('users', 'u')
      ->fields('u', array('name'))
      ->condition('uid', $row->uid, '=')
      ->execute()
      ->fetchAssoc();
    $destination = drupal_get_destination();
    $rows[] = array(
      'data' => array(
        EXAM_SPIDER_EXAM_TITLE . '-' . $row->id,
        l(filter_xss($row->exam_name), EXAM_SPIDER_EXAM_URL . '/' . $row->id . '/continue'),
        filter_xss($row->exam_description),
        $user_data['name'],
        $status,
        l(t('Questions'), 'admin/structure/question/add/' . $row->id) . ' | ' . l(t('Edit'), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/' . $row->id . '/edit', array('query' => array($destination))) . ' | ' . l(t('Delete'), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/' . $row->id . '/delete', array('query' => array($destination))),
      ),
    );
    $i++;
  }
  $output .= theme('table', array(
    'header' => $header,
    'rows'   => $rows,
    'empty'  => t('No @examSpiderExamTitle created yet.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)) . l(t('+ Create @examSpiderExamTitle', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/add'),
  ));
  $output .= theme('pager');
  return ($output);
}

/**
 * Add exam page callbacks.
 */
function exam_spider_add_exam() {
  $values = array();
  return drupal_get_form('exam_spider_exam_form', $values);
}

/**
 * Edit exam page callbacks.
 */
function exam_spider_edit_exam($exam_id) {
  $values = exam_spider_get_exam($exam_id);
  return drupal_get_form('exam_spider_exam_form', $values);
}

/**
 * Exam Add/Update form.
 */
function exam_spider_exam_form($form, &$form_state, $values) {
  $form = array();
  if ($values) {
    $form['exam_id'] = array('#type' => 'value', '#value' => $values['id']);
  }
  $form['exam_name'] = array(
    '#title'         => t('@examSpiderExamTitle Name', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#type'          => 'textfield',
    '#maxlength'     => '170',
    '#required'      => TRUE,
    '#default_value' => isset($values['exam_name']) ? $values['exam_name'] : NULL,
  );
  $form['exam_description'] = array(
    '#title'         => t('Description'),
    '#type'          => 'textarea',
    '#maxlength'     => '550',
    '#cols'          => 20,
    '#rows'          => 1,
    '#default_value' => isset($values['exam_description']) ? $values['exam_description'] : NULL,
  );
  $form['examsettings'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('@examSpiderExamTitle settings', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,

  );
  $form['examsettings']['exam_duration'] = array(
    '#title'         => t('@examSpiderExamTitle Duration', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#description'   => t('@examSpiderExamTitle time duration in minutes.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#type'          => 'textfield',
    '#maxlength'     => '10',
    '#size'          => 10,
    '#required'      => TRUE,
    '#default_value' => isset($values['exam_duration']) ? $values['exam_duration'] : 0,
  );
  $form['examsettings']['total_marks'] = array(
    '#title'            => t('Total Marks'),
    '#type'             => 'textfield',
    '#maxlength'        => '10',
    '#size'             => 10,
    '#required'         => TRUE,
    '#default_value'    => isset($values['total_marks']) ? $values['total_marks'] : NULL,
    '#element_validate' => array('element_validate_integer_positive'),
  );
  $form['examsettings']['random_quest'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Display Random Question'),
    '#default_value' => isset($values['random_quest']) ? $values['random_quest'] : NULL,
  );
  $form['examsettings']['status'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Published'),
    '#default_value' => isset($values['status']) ? $values['status'] : NULL,
  );
  $form['examsettings']['negative_mark'] = array(
    '#type'          => 'checkbox',
    '#title'         => t('Negative Marking'),
    '#default_value' => isset($values['negative_mark']) ? $values['negative_mark'] : NULL,
  );
  $form['examsettings']['negative_mark_per'] = array(
    '#title'         => t('Negative mark %'),
    '#description'   => t('@examSpiderExamTitle negative marking in %.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#type'          => 'textfield',
    '#maxlength'     => '10',
    '#size'          => 10,
    '#default_value' => isset($values['negative_mark_per']) ? $values['negative_mark_per'] : NULL,
  );
  $form['examsettings']['re_attempt'] = array(
    '#title'         => t('Re-attempt @examSpiderExamTitle time', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#description'   => t('Re-attempt @examSpiderExamTitle time in days.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#type'          => 'textfield',
    '#maxlength'     => '10',
    '#size'          => 10,
    '#default_value' => isset($values['re_attempt']) ? $values['re_attempt'] : NULL,
  );
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

/**
 * Add/Update exam page validate callbacks.
 */
function exam_spider_exam_form_validate($form, &$form_state) {
  if ($form_state['values']['exam_duration'] !== '' && (!is_numeric($form_state['values']['exam_duration']) || intval($form_state['values']['exam_duration']) != $form_state['values']['exam_duration'] || $form_state['values']['exam_duration'] < 0)) {
    form_set_error('exam_duration', t('@examSpiderExamTitle duration must be a positive integer.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)));
  }
  if ($form_state['values']['negative_mark_per'] !== '' && (!is_numeric($form_state['values']['negative_mark_per']) || intval($form_state['values']['negative_mark_per']) != $form_state['values']['negative_mark_per'] || $form_state['values']['negative_mark_per'] < 0)) {
    form_set_error('negative_mark_per', t('@examSpiderExamTitle negative mark must be a positive integer.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)));
  }
  if ($form_state['values']['re_attempt'] !== '' && (!is_numeric($form_state['values']['re_attempt']) || intval($form_state['values']['re_attempt']) != $form_state['values']['re_attempt'] || $form_state['values']['re_attempt'] < 0)) {
    form_set_error('re_attempt', t('Re-attempt @examSpiderExamTitle time must be a positive integer.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)));
  }
}

/**
 * Exam Add/Update form submit callbacks.
 */
function exam_spider_exam_form_submit($form, &$form_state) {
  global $user;
  if (arg(3) == 'add') {
    db_insert('exam_list')
      ->fields(array(
        'exam_name',
        'exam_description',
        'uid',
        'exam_duration',
        'total_marks',
        'random_quest',
        'negative_mark',
        'negative_mark_per',
        're_attempt',
        'status',
        'created',
        'changed',
      ))
      ->values(array(
        'exam_name'         => $form_state['values']['exam_name'],
        'exam_description'  => $form_state['values']['exam_description'],
        'uid'               => $user->uid,
        'exam_duration'     => $form_state['values']['exam_duration'],
        'total_marks'       => $form_state['values']['total_marks'],
        'random_quest'      => $form_state['values']['random_quest'],
        'negative_mark'     => $form_state['values']['negative_mark'],
        'negative_mark_per' => (int) $form_state['values']['negative_mark_per'],
        're_attempt'        => (int) $form_state['values']['re_attempt'],
        'status'            => $form_state['values']['status'],
        'created'           => REQUEST_TIME,
        'changed'           => REQUEST_TIME,
      ))
      ->execute();

    drupal_set_message(t('You have successfully created @examSpiderExamTitle, Please add Questions for @examName @examSpiderExamTitle.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE, '@examName' => $form_state['values']['exam_name'])));
  }
  else {
    db_update('exam_list')
      ->fields(array(
        'exam_name'         => $form_state['values']['exam_name'],
        'exam_description'  => $form_state['values']['exam_description'],
        'uid'               => $user->uid,
        'exam_duration'     => $form_state['values']['exam_duration'],
        'total_marks'       => $form_state['values']['total_marks'],
        'random_quest'      => $form_state['values']['random_quest'],
        'negative_mark'     => $form_state['values']['negative_mark'],
        'negative_mark_per' => (int) $form_state['values']['negative_mark_per'],
        're_attempt'        => (int) $form_state['values']['re_attempt'],
        'status'            => $form_state['values']['status'],
        'changed'           => REQUEST_TIME,
      ))
      ->condition('id', $form_state['values']['exam_id'], '=')
      ->execute();
    drupal_set_message(t('You have successfully updated @examName @examSpiderExamTitle.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE, '@examName' => $form_state['values']['exam_name'])));
  }
  $form_state['redirect'] = 'admin/structure/' . EXAM_SPIDER_EXAM_URL;
}

/**
 * Delete exam confirm page callbacks.
 */
function exam_spider_exam_delete_confirm($form, &$form_state, $exam_id) {
  $form['exam_id'] = array('#type' => 'value', '#value' => $exam_id);
  $exam_data = exam_spider_get_exam($exam_id);

  $path = 'admin/structure/' . EXAM_SPIDER_EXAM_URL;
  return confirm_form($form,
    t('Are you sure you want to delete @exam_name ?', array('@exam_name' => $exam_data['exam_name'])),
    $path,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete exam confirm submit page callbacks.
 */
function exam_spider_exam_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $exam_id = $form_state['values']['exam_id'];
    $exam_data = exam_spider_get_exam($exam_id);
    db_delete('exam_list')
      ->condition('id', $exam_id, '=')
      ->execute();
    db_delete('exam_questions')
      ->condition('examid', $exam_id, '=')
      ->execute();

    drupal_set_message(t('@exam_name has been deleted successfully.', array('@exam_name' => $exam_data['exam_name'])));
  }
}

/**
 * All exam listed page to start exam page callbacks.
 */
function exam_spider_exam_start() {
  $output = NULL;
  $header = array(
    array('data' => EXAM_SPIDER_EXAM_TITLE . ' Name', 'field' => 'exam_name'),
    array(
      'data'  => EXAM_SPIDER_EXAM_TITLE . ' Description',
      'field' => 'exam_description',
    ),
    array('data' => "Operations"),

  );
  $query = db_select("exam_list", "a")
    ->extend('PagerDefault')
    ->limit(10)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields("a", array(
      'id',
      'exam_name',
      'exam_description',
      'status',
    ));
  $results = $query->execute();
  $rows = array();
  $i = 0;
  foreach ($results as $row) {
    if ($row->status == 1) {
      $rows[] = array(
        'data' => array(
          l(t('@examName', array('@examName' => filter_xss($row->exam_name))), EXAM_SPIDER_EXAM_URL . '/' . $row->id . '/continue'),
          filter_xss($row->exam_description),
          l(t('Start @examSpiderExamTitle', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)), EXAM_SPIDER_EXAM_URL . '/' . $row->id . '/continue'),
        ),
      );

    }
    $i++;
  }
  $output .= theme('table', array(
    'header' => $header,
    'rows'   => $rows,
    'empty'  => t('No @examSpiderExamTitle created yet.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
  ));
  $output .= theme('pager');
  return ($output);
}

/**
 * Exam continue page callbacks.
 */
function exam_spider_exam_continue($form, &$form_state, $exam_id) {
  $form['exam_id'] = array('#type' => 'value', '#value' => $exam_id);
  $exam_data = exam_spider_get_exam($exam_id);
  $re_attempt = $exam_data['re_attempt'];
  $user_last_result = exam_spider_any_exam_last_result($exam_id);
  $user_last_attempt_timestamp = $user_last_result['created'];
  $re_attempt_timestamp = strtotime('+' . $re_attempt . ' day', $user_last_attempt_timestamp);
  if ($re_attempt_timestamp > REQUEST_TIME) {
    $form_state['storage']['re_exam_warning'] = t('You have already attempt this @examSpiderExamTitle, You will be eligible again after @re_attempt days from previus @examSpiderExamTitle attempt day.', array(
      '@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE,
      '@re_attempt' => $re_attempt,
    ));
  }
  if (isset($form_state['storage']['exam_result'])) {
    drupal_set_title(t('@examSpiderExamTitle Score', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)));
    $form['exam_result_score'] = array(
      '#markup' => $form_state['storage']['exam_result'],
    );
  }
  elseif (isset($form_state['storage']['re_exam_warning'])) {
    drupal_set_title(t('Re-attempt @examSpiderExamTitle warning', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)));
    $form['exam_result_score'] = array(
      '#markup' => $form_state['storage']['re_exam_warning'],
    );
  }
  else {
    $output = NULL;
    $form['#prefix'] = '<div id="exam_timer"></div>';
    drupal_add_css(drupal_get_path('module', EXAM_SPIDER_EXAM_SPIDER) . '/css/exam_spider.css');
    drupal_add_js(drupal_get_path('module', EXAM_SPIDER_EXAM_SPIDER) . '/js/exam_spider.js');
    if ($exam_data['status'] == 0) {
      return drupal_access_denied();
    }

    if ($exam_data['random_quest'] == 1) {
      $query = db_select("exam_questions", "eq")
        ->fields("eq")
        ->condition('examid', $exam_id, '=')->orderRandom()->execute();

    }
    else {
      $query = db_select("exam_questions", "eq")
        ->fields("eq")
        ->condition('examid', $exam_id, '=')->execute();
    }
    $results = $query->fetchAll();
    drupal_set_title(filter_xss($exam_data['exam_name']));
    if (empty($results)) {
      $output .= t('No question created yet for this @examSpiderExamTitle.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE));
    }
    else {
      if ($exam_data['exam_duration'] > 0) {
        exam_spider_clock('exam-spider-exam-continue');
      }
      $form['li_prefix'] = array(
        '#markup' => ' <ul class="exam_spider_slider_exam">',
      );
      $total_slides = count($results);
      foreach ($results as $key => $value) {
        $options[1] = filter_xss($value->opt1);
        $options[2] = filter_xss($value->opt2);
        $options[3] = filter_xss($value->opt3);
        $options[4] = filter_xss($value->opt4);
        if ($value->multiple == 1) {
          $form['question'][$value->id] = array(
            '#type'    => 'checkboxes',
            '#options' => $options,
            '#title'   => t('@question', array('@question' => filter_xss($value->question))),
            '#prefix'  => '<li id="examslide_' . $key . '" class="exam_spider_slider">',
            '#suffix'  => ' <a class="exam_spider_slide_next button" href="javascript:void(0);">' . t('Next') . '</a></li>',
          );
        }
        else {
          $form['question'][$value->id] = array(
            '#type'    => 'radios',
            '#title'   => t('@question', array('@question' => filter_xss($value->question))),
            '#options' => $options,
            '#prefix'  => '<li id="examslide_' . $key . '" class="exam_spider_slider">',
            '#suffix'  => ' <a class="exam_spider_slide_next button" href="javascript:void(0);">' . t('Next') . '</a></li>',
          );
        }
      }
      $form['next'] = array(
        '#type'   => 'submit',
        '#prefix' => '<li id="examslide_' . $total_slides . '" class="exam_spider_slider">' . t('<h2>I am done.</h2><br />'),
        '#suffix' => '</li>',
        '#value'  => t('Submit'),
      );
      $form['#tree'] = TRUE;
      $form['li_suffix'] = array(
        '#markup' => '</ul>',
      );
    }
    $form['#suffix'] = $output;
  }
  $form['#redirect'] = FALSE;
  return $form;
}

/**
 * Exam continue submit page callbacks.
 */
function exam_spider_exam_continue_submit(&$form, &$form_state) {
  global $user;
  $score_obtain = $total_marks = $wrong_quest = 0;
  $exam_data = exam_spider_get_exam($form_state['values']['exam_id']);
  $total_marks = $exam_data['total_marks'];
  $negative_mark = $exam_data['negative_mark'];
  $negative_mark_per = $exam_data['negative_mark_per'];
  $total_quest = count($form_state['values']['question']);
  $mark_per_quest = ($total_marks / $total_quest);
  $re_attempt = $exam_data['re_attempt'];
  $user_last_result = exam_spider_any_exam_last_result($form_state['values']['exam_id']);
  $user_last_attempt_timestamp = $user_last_result['created'];
  $re_attempt_timestamp = strtotime('+' . $re_attempt . ' day', $user_last_attempt_timestamp);
  if ($re_attempt_timestamp > REQUEST_TIME) {
    $form_state['storage']['re_exam_warning'] = t('You have already attempt this @examSpiderExamTitle, You will be eligible again after @re_attempt days from previus @examSpiderExamTitle attempt day.', array(
      '@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE,
      '@re_attempt' => $re_attempt,
    ));
  }
  else {
    $negative_marking_number = (($mark_per_quest * $negative_mark_per) / 100);
    foreach ($form_state['values']['question'] as $key => $answervalues) {
      $question_data = exam_spider_get_question($key);
      if (is_array($answervalues)) {
        $answer_combine = '';
        foreach ($answervalues as $key => $answervalue) {
          if ($answervalue != 0) {
            $answer_combine .= 'opt' . $answervalue . '-';
          }
        }
        $checkanswer = rtrim($answer_combine, "-");
        if ($checkanswer == $question_data['answer']) {
          $score_obtain += $mark_per_quest;
        }
        else {
          if ($negative_mark == 1) {
            $score_obtain -= $negative_marking_number;
          }
          $wrong_quest += 1;
        }
      }
      else {
        $checkanswer = 'opt' . $answervalues;
        if ($checkanswer == $question_data['answer']) {
          $score_obtain += $mark_per_quest;
        }
        else {
          if ($negative_mark == 1) {
            $score_obtain -= $negative_marking_number;
          }
          $wrong_quest += 1;
        }
      }
    }
    $correct_answers = $total_quest - $wrong_quest;
    $form_state['storage']['exam_result'] = t('<b>You have got @score_obtain marks out of @total_marks<br/>Correct Answer(s) @correctAnswers <br/>Wrong Answer(s) @wrong_quest<b>', array(
      '@score_obtain' => $score_obtain,
      '@total_marks' => $total_marks,
      '@correctAnswers' => $correct_answers,
      '@wrong_quest' => $wrong_quest,
    ));
    $reg_id = db_insert('exam_results')
      ->fields(array('examid', 'uid', 'total', 'obtain', 'wrong', 'created'))
      ->values(array(
        'examid'  => $form_state['values']['exam_id'],
        'uid'     => $user->uid,
        'total'   => $total_marks,
        'obtain'  => $score_obtain,
        'wrong'   => $wrong_quest,
        'created' => REQUEST_TIME,
      ))
      ->execute();
    drupal_set_message(t('Your @examSpiderExamTitle has submitted successfully and your REG id is REG-@reg_id.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE, '@reg_id' => $reg_id)));
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Exam results page callbacks.
 */
function exam_spider_exam_results($form, &$form_state) {
  drupal_add_css(drupal_get_path('module', EXAM_SPIDER_EXAM_SPIDER) . '/css/exam_spider.css');
  $output = NULL;
  $exam_names = array();
  $exams_data = exam_spider_get_exam();

  foreach ($exams_data as $exam_name) {
    $exam_names[$exam_name->id] = $exam_name->exam_name;
  }
  $form['#method'] = 'get';
  $form['filter'] = array(
    '#type'        => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
    '#title'       => t('Filter option'),
    '#attributes'  => array('class' => array('container-inline')),
  );

  $form['filter']['exam_name'] = array(
    '#type'          => 'select',
    '#title'         => t('@examSpiderExamTitle Name', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#options'       => $exam_names,
    '#default_value' => isset($_GET['exam_name']) ? $_GET['exam_name'] : NULL,
  );

  $form['filter']['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Filter'),
  );
  $form['filter']['reset_button'] = array(
    '#markup' => l(t('Reset'), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/results', array('attributes' => array('class' => array('button')))),
  );

  $header = array(
    array('data' => 'REG Id', 'field' => 'id', 'sort' => 'desc'),
    array('data' => EXAM_SPIDER_EXAM_TITLE . ' Name', 'field' => 'examid'),
    array('data' => 'Name', 'field' => 'uid'),
    array('data' => 'Total Marks', 'field' => 'total'),
    array('data' => 'Obtain Marks', 'field' => 'obtain'),
    array('data' => 'Wrong', 'field' => 'wrong'),
    array('data' => 'Date', 'field' => 'created'),
    array('data' => "Operations"),
  );
  $query = db_select("exam_results", "er")
    ->extend('PagerDefault')
    ->limit(10)
    ->extend('TableSort')
    ->orderByHeader($header)
    ->fields("er", array(
      'id',
      'examid',
      'uid',
      'total',
      'obtain',
      'wrong',
      'created',
    ));
  if (isset($_GET['exam_name'])) {
    $query->condition('examid', $_GET['exam_name']);
  }
  $results = $query->execute();
  $rows = array();
  $i = 0;
  foreach ($results as $row) {
    $user_data = db_select('users', 'u')
      ->fields('u', array('name'))
      ->condition('uid', $row->uid, '=')
      ->execute()
      ->fetchAssoc();
    $exam_data = exam_spider_get_exam($row->examid);
    $destination = drupal_get_destination();
    $rows[] = array(
      'data' => array(
        t('REG -') . $row->id,
        filter_xss_admin($exam_data['exam_name']),
        $user_data['name'],
        $row->total,
        $row->obtain,
        $row->wrong,
        format_date($row->created, 'short'),
        l(t('Delete'), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/result/' . $row->id . '/delete', array('query' => array($destination))) . ' | ' . l(t('Send Mail'), 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/result/' . $row->examid . '/' . $row->uid . '/mail'),
      ),
    );
    $i++;
  }
  $form['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No @examSpiderExamTitle result found.', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
  );
  $form['pager'] = array('#markup' => theme('pager'));
  $form['results_table'] = array(
    '#markup' => $output,
  );
  return $form;
}

/**
 * Exam clock function.
 */
function exam_spider_clock($formid) {
  require_once drupal_get_path('module', EXAM_SPIDER_EXAM_SPIDER) . '/includes/ExamSpiderDigitalClock.php';
  $clock = new ExamSpiderDigitalClock();
  return $clock->showClock($formid);
}

/**
 * Delete exam result confirm page callbacks.
 */
function exam_spider_exam_result_delete_confirm($form, &$form_state, $result_id) {
  $form['result_id'] = array('#type' => 'value', '#value' => $result_id);
  $path = 'admin/structure/' . EXAM_SPIDER_EXAM_URL . '/results';
  return confirm_form($form,
    t('Are you sure you want to delete REG - @exam_name  result ?', array('@exam_name' => $result_id)),
    $path,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete exam confirm submit page callbacks.
 */
function exam_spider_exam_result_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $result_id = $form_state['values']['result_id'];
    db_delete('exam_results')
      ->condition('id', $result_id, '=')
      ->execute();
    drupal_set_message(t('REG - @exam_name result has been deleted successfully.', array('@exam_name' => $result_id)));
  }
}

/**
 * Send result score card via mail.
 */
function exam_spider_exam_result_mail($exam_id, $to_uid) {
  $exam_result_data = exam_spider_any_exam_last_result($exam_id, $to_uid);
  $user_data = user_load($to_uid);
  $exam_data = exam_spider_get_exam($exam_id);

  $body = t('Hi @tomail,

You have got @score_obtain marks out of @total_marks.
Wrong Answer(s) @wrong_quest.

Many Thanks,
@sitename', array(
  '@score_obtain'   => $exam_result_data['obtain'],
  '@total_marks'    => $exam_result_data['total'],
  '@wrong_quest'    => $exam_result_data['wrong'],
  '@sitename'       => variable_get('site_name', 'Drupal'),
  '@tomail'         => @$user_data->name,

));

  $module = 'exam_spider';
  $my_mail_token = 'exam_spider_result';
  $from = variable_get('site_mail', 'admin@example.com');
  $to = $user_data->mail;
  $message = array(
    'id' => $module . '_' . $my_mail_token,
    'to' => $to,
    'subject' => 'Eaxam Result for ' . $exam_data['exam_name'],
    'body' => array($body),
    'headers' => array(
      'From' => $from,
      'Sender' => $from,
      'Return-Path' => $from,
    ),
  );
  $system = drupal_mail_system($module, $my_mail_token);
  // The format function must be called before calling the mail function.
  $message = $system->format($message);

  if ($system->mail($message)) {
    drupal_set_message(t('Score card mail has been sent.'));
  }
  else {
    drupal_set_message(t('There was a problem sending your mail and it was not sent.'), 'error');
  }
  drupal_goto('admin/structure/' . EXAM_SPIDER_EXAM_URL . '/results');
}

/**
 * Exam configuration form callbacks.
 */
function exam_spider_exam_config($form, &$form_state) {
  $form['exam_spider_exam_name'] = array(
    '#type' => 'textfield',
    '#title' => t('@examSpiderExamTitle Name', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#default_value' => EXAM_SPIDER_EXAM_TITLE,
    '#description' => t('Please enter exam name to update'),
    '#required' => TRUE,
  );
  $form['exam_spider_exam_url'] = array(
    '#type' => 'textfield',
    '#title' => t('@examSpiderExamTitle URL', array('@examSpiderExamTitle' => EXAM_SPIDER_EXAM_TITLE)),
    '#default_value' => EXAM_SPIDER_EXAM_URL,
    '#description' => t('Please enter exam URL to update'),
    '#required' => TRUE,
  );
  $form = system_settings_form($form);
  $form['#submit'][] = 'exam_spider_exam_config_submit';
  return $form;
}

/**
 * Exam configuration form validate callbacks.
 */
function exam_spider_exam_config_validate($form, &$form_state) {
  $exam_spider_exam_name = $form_state['values']['exam_spider_exam_name'];
  $exam_spider_exam_url = $form_state['values']['exam_spider_exam_url'];
  if (preg_match('/[^a-z]+$/i', $exam_spider_exam_name)) {
    form_set_error('exam_spider_exam_name', t('Please use only charcters to update name.'));
  }
  elseif (preg_match('/[^a-z]+$/', $exam_spider_exam_url)) {
    form_set_error('exam_spider_exam_url', t('Please use only lowercase charcters to update path.'));
  }
  $updated_path = 'admin/structure/' . $exam_spider_exam_url;
  $exposed_args_count = count(explode('/', $updated_path));
  $menu_item = menu_get_item($updated_path);
  if (($menu_item['number_parts'] == $exposed_args_count) && drupal_valid_path($updated_path) && ($exam_spider_exam_url != EXAM_SPIDER_EXAM_URL)) {
    form_set_error('exam_spider_exam_url', t('A path already exists for the source path @source.', array('@source' => $updated_path)));
  }
  else {
    variable_set('menu_rebuild_needed', TRUE);
  }
}

/**
 * Exam configuration form submit callbacks.
 */
function exam_spider_exam_config_submit($form, &$form_state) {
  if (isset($form_state['values']['exam_spider_exam_url'])) {
    drupal_goto('admin/structure/' . $form_state['values']['exam_spider_exam_url']);
  }
}
