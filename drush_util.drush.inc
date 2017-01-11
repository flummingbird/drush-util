<?php

function drush_util_drush_command(){
    $items['create-node'] = array(
        'description' => 'Create a node.',
        'options' => array(
            'title' => array(
                'description' => 'Node title.',
                'required' => 'TRUE',
            ),
            'type' => array(
                'description' => 'Node type.',
                'required' => TRUE,
            ),
            'body' => array(
                'description' => "Node body.",
                'required' => TRUE,
            ),
            'path' => array(
                'description' => "Node path alias.",
                'required' => TRUE,
            ),
        ),
        'aliases' => array('du-cn'),
        'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL, // No bootstrap at all.
    );
    $items['create-node-csv'] = array(
        'description' => 'Create nodes from CSV.',
        'options' => array(
            'file' => array(
                'description' => 'Path to CSV.',
                'required' => 'TRUE',
            ),
        ),
        'aliases' => array('du-cn-csv'),
        'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL, // No bootstrap at all.
    );
    return $items;
}

function drush_drush_util_create_node_csv(){
    $rows = array();
    if(($handle = fopen(drush_get_option('file'), 'r')) !== false){
        $fields = fgetcsv($handle);
        while(($data = fgetcsv($handle)) !== false){
            $rows[] = (object)array_combine($fields, $data);
        }
        fclose($handle);
    }
    if(empty($rows)){
        drupal_set_message(t("No rows found in file."));
        return false;
    }
    foreach($rows as $node){
        create_node($node->type, $node->title, $node->body, $node->path);
    }
}

function drush_drush_util_create_node(){
    list($type, $title, $body, $path) = array_map('drush_get_option', array('type', 'title', 'body', 'path'));
    create_node($type, $title, $body, $path);
}

function create_node($type = 'page', $title = 'test', $body = 'hello world', $path = null){
    $node = new stdClass();  // Create a new node object
    $node->language = LANGUAGE_NONE;  // Or e.g. 'en' if locale is enabled
    $node->title = $title;
    $node->body[$node->language][0]['value'] = $body;
    $node->type = $type;  // Content type

    if($path) {
      $node->path['pathauto'] = 0; // Required only if Pathauto is enabled.
      $node->path['alias'] = $path;
    }

    $nid = node_exists($node);
    
    if($nid){
        $node->nid = $nid;
        drupal_set_message(t("Node already exists, updating."));
        return node_save($node);
    }
    node_object_prepare($node);  //Set some default values

    $node->status = 1;   // (1 or 0): published or unpublished
    $node->promote = 0;  // (1 or 0): promoted to front page or not
    $node->sticky = 0;  // (1 or 0): sticky at top of lists or not
    $node->comment = 1;  // 2 = comments open, 1 = comments closed, 0 = comments hidden
    // Add author of the node
    $node->uid = 1;

    if($type == 'book'){
      // Set book-specific attributes
      $node->book = array();
      $node->book['bid'] = "new";
      $node->book['has_children'] = 0;
      $node->book['module'] = 'book';
    }
    // Save the node
    node_save($node);
}

function node_exists(stdClass $object){
    $query  = "SELECT nid FROM node WHERE type = :type AND title = :title";
    $args   = array(':type' => $object->type, ':title' => $object->title);
    $result = db_query($query, $args);

    while($r = $result->fetchAssoc()){
        return $r['nid'];
    }
    return false;
}