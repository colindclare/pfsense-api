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

class APIServicesAcmeCertificateCreate extends APIModel {

    private $current_certs;
    private $cert_defaults;

    # Create our method constructor
    public function __construct() {
        global $a_acmeserver;

        parent::__construct();
        $this->packages = ["pfSense-pkg-acme"];
        $this->privileges = ["page-all", "page-service-acme"];
        $this->change_note = "Created ACME account key via API.";
        $this->current_certs = $this->get_config("installedpackages/acme/certificates/item", []);
        $this->cert_defaults = [
            "status" => "active",
            "ocspstaple" => "",
            "preferredchain" => "",
            "dnssleep" => "",
            "renewafter" => "",
            "a_actionlist" => [
                "item" => [
                    [
                        'status' => 'enable',
                        'method' => 'shellcommand',
                        'shellcommand' => '/usr/local/etc/rc.d/haproxy.sh restart'
                    ]
                ]
            ]
        ];

    }

    public function validate_payload() {
        if ($this->__validate_name()) {
            $this->validated_data['name'] = trim($this->initial_data['name']);
        }  
        $this->__validate_descr();
        $this->__validate_domains();
        $this->__validate_account_key();
        $this->__set_key_length();
        $this->__populate_defaults();
    }

    private function __validate_descr() {
        if (!isset($this->initial_data['descr']) || empty($this->initial_data['descr'])) {
            $this->errors[] = APIResponse\get(2288, ['request_data' => $this->initial_data]);
        } else {
            $this->validated_data['descr'] = $this->initial_data['descr'];
        }
    }

    private function __validate_name() {
        $name_is_unique = true;

        /* Ensure that our key names are unique */
        foreach ($this->current_certs as $key) {
            if ($this->initial_data['name'] == $key['name']) {
                $this->errors[] = APIResponse\get(2287, ['name' => $this->initial_data['name']]);
                $name_already_used = false;
            }  
        }

        /* Ensure key name has no spaces, registration breaks otherwise */
        if (str_contains(trim($this->initial_data['name']), ' ')) {
            $this->errors[] = APIResponse\get(2291, ['name' => $this->initial_data['name']]);
            $no_spaces = false;
        } else {
            $no_spaces = true;
        }
        return ($name_is_unique && $no_spaces);
    }

    private function __validate_account_key() {
        $key_found = false;
        $existing_keys = $this->get_config('installedpackages/acme/accountkeys/item', []);
        foreach ($existing_keys as $key) {
            if ($key['name'] == $this->initial_data['acmeaccount']) {
                $this->validated_data['acmeaccount'] = $this->initial_data['acmeaccount'];
                $key_found = true;
                break;
            }
        }

        if (!$key_found) {
            $this->errors[] = APIResponse\get(2290, ['key' => $this->initial_data['acmeaccount']]);
        }
    }

    private function __validate_domains() {
        $this->validated_data['a_domainlist'] = ['item' => []];
        $bad_domains = [];
        foreach ($this->initial_data['san_domains'] as $domain) {
            if (substr($domain, 0, 2) === '*.') {
                $non_wc_domain = substr($domain, 2);
            } else {
                $non_wc_domain = $domain;
            }
            if (filter_var($non_wc_domain, FILTER_VALIDATE_DOMAIN, FILTER_FLAG_HOSTNAME)) {
                $new_domain = [];
                $new_domain['name'] = $domain;
                $new_domain['status'] = 'enable';
                $new_domain['method'] = 'standalone';
                $new_domain['standaloneport'] = '8080';
                $this->validated_data['a_domainlist']['item'][] = $new_domain;
            } else {
                $bad_domains[] = $domain;
            }
        }

        if (count($bad_domains) > 0) {
            $this->errors[] = APIResponse\get(2289, ['bad_domains' => $bad_domains]);
        }
    }

    private function __populate_defaults() {
        foreach ($this->cert_defaults as $setting => $value) {
            if (!array_key_exists($setting, $this->validated_data)) {
                $this->validated_data[$setting] = $this->cert_defaults[$setting];
            }   
        }   
    }

    private function __set_key_length() {

        if (isset($this->initial_data['use_ecdsa']) && $this->initial_data['use_ecdsa']) {
            $this->validated_data['keylength'] = 'ec-384';
        } else {
            $this->validated_data['keylength'] = '4096';
        }

    }

    public function action() {

        $this->id = $this->get_next_id("installedpackages/acme/certificates/item");
        $this->set_config("installedpackages/acme/certificates/item/{$this->id}", $this->validated_data);
        $this->write_config();
	if (isset($this->initial_data['issuecert']) && $this->initial_data['issuecert']) {
	    // Start an output buffer to keep things clean, since the acme package doesn't capture stdout/stderr
	    ob_start();
	    $result = pfsense_pkg\acme\issue_certificate($this->validated_data['name'], true);
	    ob_end_clean();
	    // End and clear output buffer
	    if (result) {
                return APIResponse\get(0, ['msg' => 'ACME Certificate successfully created', 'request_data' => $this->validated_data]);
	    } else {
		return APIResponse\get(2292, ['request_data' => $this->initial_data]);
	    }
	} else {
            return APIResponse\get(0, ['msg' => 'ACME Certificate successfully created, but not issued.', 'request_data' => $this->validated_data]);
	}
    }
}
