require 'spec_helper'
require 'test_cluster'

describe 'replication client' do |format|
  before(:context) do
    TEST_CLUSTER.start

    TEST_CLUSTER.postgres.exec('CREATE TABLE things (id SERIAL PRIMARY KEY NOT NULL)')
  end

  after(:example) do
    TEST_CLUSTER.send(:services, /bottledwater-/).each {|container| TEST_CLUSTER.send(:dump_container_logs, container) } # TODO
  end

  after(:context) do
    TEST_CLUSTER.stop
  end

  let(:postgres) { TEST_CLUSTER.postgres }

  def restart_lsn
    result = postgres.exec(%(SELECT restart_lsn FROM pg_replication_slots WHERE slot_name = 'bottledwater'))
    result[0].fetch('restart_lsn')
  end

  it 'consumes WAL after an INSERT' do
    lsn_before = restart_lsn

    postgres.exec('INSERT INTO things DEFAULT VALUES')
    sleep 20

    lsn_after = restart_lsn

    expect(lsn_after).to be > lsn_before
  end

  it 'consumes WAL after a committed transaction' do
    lsn_before = restart_lsn

    postgres.exec('BEGIN')
    postgres.exec('INSERT INTO things DEFAULT VALUES')
    postgres.exec('COMMIT')
    sleep 20

    lsn_after = restart_lsn

    expect(lsn_after).to be > lsn_before
  end

  it 'consumes WAL after a rolled back transaction' do
    lsn_before = restart_lsn

    postgres.exec('BEGIN')
    postgres.exec('INSERT INTO things DEFAULT VALUES')
    postgres.exec('ROLLBACK')
    sleep 20

    lsn_after = restart_lsn

    expect(lsn_after).to be > lsn_before
  end
end
