<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\FirewallAlias;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\RoutingGateway;
use RESTAPI\Models\StaticRoute;

class APIModelsStaticRouteTestCase extends TestCase {
    /**
     * Checks that the static route cannot conflict with an existing interface's subnet.
     */
    public function test_static_route_cannot_conflict_with_interface_subnet() {
        # Create an interface to use for testing
        $test_if = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'enable' => true,
                'descr' => 'TESTIF',
                'typev4' => 'static',
                'ipaddr' => '1.2.3.4',
                'subnet' => 24,
                'typev6' => 'staticv6',
                'ipaddrv6' => '1234::2',
                'subnetv6' => 64,
            ],
            async: false,
        );
        $test_if->create();

        # Create gateways to use for testing
        $test_gw4 = new RoutingGateway(
            data: [
                'name' => 'TESTGW4',
                'interface' => $test_if->id,
                'ipprotocol' => 'inet',
                'gateway' => '1.2.3.1',
            ],
            async: false,
        );
        $test_gw6 = new RoutingGateway(
            data: [
                'name' => 'TESTGW6',
                'interface' => $test_if->id,
                'ipprotocol' => 'inet6',
                'gateway' => '1234::1',
            ],
            async: false,
        );
        $test_gw4->create();
        $test_gw6->create();

        # Ensure an IPv4 static route cannot be created for the interface's 1.2.3.0/24 subnet.
        $this->assert_throws_response(
            response_id: 'STATIC_ROUTE_CONFLICTS_WITH_IPV4_INTERFACE',
            code: 400,
            callable: function () use ($test_if, $test_gw4) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw4->name->value,
                        'network' => $test_if->ipaddr->value . '/' . $test_if->subnet->value,
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );
        # Ensure an IPv6 static route cannot be created for the interface's 1234::/64 subnet.
        $this->assert_throws_response(
            response_id: 'STATIC_ROUTE_CONFLICTS_WITH_IPV6_INTERFACE',
            code: 400,
            callable: function () use ($test_if, $test_gw6) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw6->name->value,
                        'network' => $test_if->ipaddrv6->value . '/' . $test_if->subnetv6->value,
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );

        # Ensure routes can be created for subnets that do not conflict with any interface's subnet.
        $this->assert_does_not_throw(
            callable: function () use ($test_gw4) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw4->name->value,
                        'network' => '4.3.2.1/24',
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );
        $this->assert_does_not_throw(
            callable: function () use ($test_gw6) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw6->name->value,
                        'network' => '4321::/64',
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );

        # Remove objects created by this test
        $test_gw6->delete(apply: true);
        $test_gw4->delete(apply: true);
        $test_if->delete(apply: true);
    }

    /**
     * Checks that the static route's network matches the IP type of the gateway.
     */
    public function test_static_route_network_must_match_gateway_ip_type() {
        # Create an interface to use for testing
        $test_if = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'enable' => true,
                'descr' => 'TESTIF',
                'typev4' => 'static',
                'ipaddr' => '1.2.3.4',
                'subnet' => 24,
                'typev6' => 'staticv6',
                'ipaddrv6' => '1234::2',
                'subnetv6' => 64,
            ],
            async: false,
        );
        $test_if->create();

        # Create gateways to use for testing
        $test_gw4 = new RoutingGateway(
            data: [
                'name' => 'TESTGW4',
                'interface' => $test_if->id,
                'ipprotocol' => 'inet',
                'gateway' => '1.2.3.1',
            ],
            async: false,
        );
        $test_gw6 = new RoutingGateway(
            data: [
                'name' => 'TESTGW6',
                'interface' => $test_if->id,
                'ipprotocol' => 'inet6',
                'gateway' => '1234::1',
            ],
            async: false,
        );
        $test_gw4->create();
        $test_gw6->create();

        # Ensure an IPv4 static route cannot be created for the interface's 1.2.3.0/24 subnet.
        $this->assert_throws_response(
            response_id: 'STATIC_ROUTE_NON_IPV4_NETWORK_WITH_IPV4_GATEWAY',
            code: 400,
            callable: function () use ($test_if, $test_gw4) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw4->name->value,
                        'network' => '1111::1/64',
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );
        # Ensure an IPv6 static route cannot be created for the interface's 1234::/64 subnet.
        $this->assert_throws_response(
            response_id: 'STATIC_ROUTE_NON_IPV6_NETWORK_WITH_IPV6_GATEWAY',
            code: 400,
            callable: function () use ($test_if, $test_gw6) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw6->name->value,
                        'network' => '1.1.1.1/24',
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );

        # Remove objects created by this test
        $test_gw6->delete(apply: true);
        $test_gw4->delete(apply: true);
        $test_if->delete(apply: true);
    }

    /**
     * Checks that static routes must be disabled if the parent gateway is disabled.
     */
    public function test_static_route_must_be_disabled_when_gateway_is_disabled() {
        # Create gateways to use for testing
        $test_gw4 = new RoutingGateway(
            data: [
                'name' => 'TESTGW4',
                'interface' => 'lan',
                'ipprotocol' => 'inet',
                'gateway' => '1.2.3.1',
                'disabled' => true,
            ],
            async: false,
        );
        $test_gw4->create();

        # Ensure an IPv4 static route cannot be created for the interface's 1.2.3.0/24 subnet.
        $this->assert_throws_response(
            response_id: 'STATIC_ROUTE_GATEWAY_DISABLED',
            code: 400,
            callable: function () use ($test_gw4) {
                $test_route = new StaticRoute(
                    data: [
                        'gateway' => $test_gw4->name->value,
                        'network' => '4.3.2.1/24',
                        'disabled' => false,
                    ],
                    async: false,
                );
                $test_route->validate();
            },
        );

        $test_gw4->delete(apply: true);
    }

    /**
     * Checks that the `get_networks()` method correctly gathers all applicable networks for the given `network` value.
     */
    public function test_get_networks() {
        # Create an alias to use for testing
        $test_alias = new FirewallAlias(
            data: [
                'name' => 'TESTALIAS',
                'type' => 'network',
                'address' => ['192.168.10.1/24', '192.168.20.1/25', '192.168.30.0/29'],
            ],
        );
        $test_alias->create();

        # Create a gateway to use for testing
        $test_gw = new RoutingGateway(
            data: [
                'name' => 'TESTGW',
                'interface' => 'lan',
                'ipprotocol' => 'inet',
                'gateway' => '192.168.1.10',
            ],
            async: false,
        );
        $test_gw->create();

        # Create a static route to test with
        $test_route = new StaticRoute(
            data: [
                'gateway' => $test_gw->name->value,
                'network' => $test_alias->name->value,
            ],
            async: false,
        );

        # Validate the route and check that the output of get_networks() is expected
        $test_route->validate();
        $this->assert_equals($test_route->get_networks(), $test_alias->address->value);

        # Update the alias to be a host alias and check that the output of get_networks() is expected
        $test_alias->type->value = 'host';
        $test_alias->address->value = ['192.168.20.2', '192.168.30.3', '192.168.40.4'];
        $test_alias->update();
        $this->assert_equals($test_route->get_networks(), ['192.168.20.2/32', '192.168.30.3/32', '192.168.40.4/32']);

        # Ensure when `network` is set to a subnet instead of an alias, that get_networks() just returns the subnet
        $test_route->network->value = '192.168.50.0/24';
        $test_route->validate();
        $this->assert_equals($test_route->get_networks(), ['192.168.50.0/24']);

        # Delete objects created for this test
        $test_alias->delete(apply: true);
        $test_gw->delete(apply: true);
    }

    /**
     * Checks that static routes are found in the routing table after creation, updates and not found after deletion.
     */
    public function test_static_route_configured_in_routing_table() {
        # Create a gateway to use for testing
        $test_gw = new RoutingGateway(
            data: [
                'name' => 'TESTGW',
                'interface' => 'lan',
                'ipprotocol' => 'inet',
                'gateway' => '192.168.1.10',
            ],
            async: false,
        );
        $test_gw->create();

        # Create a static route using the gateway above
        $test_route = new StaticRoute(
            data: ['network' => '1.2.3.0/24', 'gateway' => $test_gw->name->value],
            async: false,
        );
        $test_route->create(apply: true);

        # Ensure the route is found in the routing table after applying
        $this->assert_str_contains(
            $this->run_command('netstat -rn', trim_whitespace: true)['output'],
            "{$test_route->network->value} {$test_gw->gateway->value}",
        );

        # Update the static route to change the static route network
        $test_route->network->value = '4.3.2.0/31';
        $test_route->update(apply: true);

        # Ensure the new route is found in the routing table and the old one is not after applying
        $this->assert_str_does_not_contain(
            $this->run_command('netstat -rn', trim_whitespace: true)['output'],
            "1.2.3.0/24 {$test_gw->gateway->value}",
        );
        $this->assert_str_contains(
            $this->run_command('netstat -rn', trim_whitespace: true)['output'],
            "{$test_route->network->value} {$test_gw->gateway->value}",
        );

        # Remove the static route and ensure it is no longer present in the routing table
        $test_route->delete(apply: true);

        $this->assert_str_does_not_contain(
            $this->run_command('netstat -rn', trim_whitespace: true)['output'],
            "{$test_route->network->value} {$test_gw->gateway->value}",
        );

        # Delete the gateway used for testing
        $test_gw->delete(apply: true);
    }

    /**
     * Checks that we can update a newly created static route that has not yet been applied. Regression test for #654.
     */
    public function test_update_static_route_before_its_applied(): void {
        # Create a gateway to use for testing
        $test_gw = new RoutingGateway(
            name: 'TESTGW',
            interface: 'lan',
            ipprotocol: 'inet',
            gateway: '192.168.1.10',
            async: false,
        );
        $test_gw->create();

        # Create a static route using the gateway above, but do not apply it!
        $test_route = new StaticRoute(gateway: $test_gw->name->value, network: '1.2.3.4/32');
        $test_route->create();

        # Ensure we can update the static route before applying it
        $test_route->network->value = '4.3.2.1/32';
        $test_route->update();

        # Remove the static route and gateway
        $test_route->delete();
        $test_gw->delete(apply: true);
    }
}
