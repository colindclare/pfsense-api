<?php
//   Copyright 2023 Colin Clare
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");
require_once("acme/acme.inc");
require_once("acme/acme_utils.inc");

class APIServicesAcmeAccountkeyCreate extends APIModel {

    private $current_keys;
    private $key_defaults;
    private $allowed_cas;

    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->packages = ["pfSense-pkg-acme"];
        $this->privileges = ["page-all", "page-service-acme"];
        $this->change_note = "Created ACME account key via API.";
        $this->current_keys = $this->get_config("installedpackages/acme/accountkeys/item", []);
        $this->allowed_cas = $a_acmeserver;
        $this->key_defaults = [
            "email" => "pfsense-le-notifications@liquidweb.com",
            "renewafter" => ""
        ];

    }

    public function validate_payload() {
        if ($this->__validate_name()) {
            $this->validated_data['name'] = $this->initial_data['name'];
        }  
        $this->__set_acme_endpoint();
        $this->__validate_descr();
        $this->__populate_defaults();
    }

    private function __validate_descr() {
        if (!isset($this->initial_data['descr']) || empty($this->initial_data['descr'])) {
            $this->errors[] = APIResponse\get(2284, ['request_data' => $this->initial_data]);
        } else {
            $this->validated_data['descr'] = $this->initial_data['descr'];
        }
    }

    private function __validate_name() {
        $name_is_unique = true;

        /* Ensure that our key names are unique */
        foreach ($this->current_keys as $key) {
            if ($this->initial_data['name'] == $key['name']) {
                $this->errors[] = APIResponse\get(2287, ['name' => $this->initial_data['name']]);
                $name_already_used = false;
            }  
        }

        return $name_is_unique;

    }

    private function __populate_defaults() {

        foreach ($this->frontend_defaults as $setting => $value) {
            if (!array_key_exists($setting, $this->validated_data)) {
                $this->validated_data[$setting] = $this->frontend_defaults[$setting];
            }   
        }   

    }

    private function __set_acme_endpoint() {

        if (isset($this->initial_data['acmeserver']) && array_key_exists($this->initial_data['acmeserver'], $this->allowed_cas)) {
            $this->validated_data['acmeserver'] = $this->initial_data['acmeserver'];
        } elseif ($this->initial_data['use_prod']) {
            $this->validated_data['acmeserver'] = "letsencrypt-production-2";
        } else { 
            $this->validated_data['acmeserver'] = "letsencrypt-staging-2";
        }

    }

    public function action() {       

        $account_key = generateAccountKey($name, $this->allowed_cas[$this->validated_data['acmeserver']]);
        $this->validated_data['accountkey'] = base64_encode($account_key);

        return APIResponse\get(0, ['msg' => 'Account key successfully created', 'request_data' => $this->validated_data]);

    }
}
