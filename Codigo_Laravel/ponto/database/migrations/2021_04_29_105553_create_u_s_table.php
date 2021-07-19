<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

class CreateUSTable extends Migration
{
    /**
     * Run the migrations.
     *
     * @return void
     */
    public function up()
    {
        Schema::create('u', function (Blueprint $table) {
            $table->bigIncrements('id');
            $table->string('u', 150)->unique();
            $table->foreign('u')->references('name')->on('users')->onUpdate('cascade');
            $table->boolean('s');
            $table->timestampTz('added_at', $precision = 0);
        });
    }

    /**
     * Reverse the migrations.
     *
     * @return void
     */
    public function down()
    {
        Schema::dropIfExists('u');
    }
}
